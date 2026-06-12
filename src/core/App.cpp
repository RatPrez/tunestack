#include "core/App.hpp"

#include <filesystem>
#include <thread>

#include <SDL3/SDL.h>

App::App(SDL_Renderer* renderer)
    : m_ytDlp(m_settings.configDir())
    , m_lastfm(m_settings.get<std::string>("lastfm_api_key"))
    , m_playlists(m_settings.configDir())
    , m_mediaManager(m_settings.configDir())
    , m_playQueue(m_player, m_mediaManager)
    , m_radio(m_player, m_mediaManager, m_lastfm)
{
    EventBus::m_instance = &m_eventBus;
    m_ytDlp.ensureAvailable();

    m_eventBus.on<std::string>("search", [this](const std::string& query) {
        std::thread([this, query]() { search(query); }).detach();
    });

    m_eventBus.on("open_settings", [this]() {
        m_settingsModal.open();
    });

    m_eventBus.on("show_liked_songs", [this]() {
        m_mainView.showLikedSongs();
    });

    // restore last session
    m_player.setVolume(m_settings.get<float>("last_volume", 0.7f));

    const std::string lastPath = m_settings.get<std::string>("last_file");
    if (!lastPath.empty() && std::filesystem::exists(lastPath))
    {
        m_player.load(lastPath);
        m_player.setPosition(m_settings.get<float>("last_position", 0.0f));
        m_player.pause();
    }
}

App::~App()
{
    m_settings.set("last_volume",   m_player.getVolume());
    m_settings.set("last_file",     m_player.getCurrentFilePath());
    m_settings.set("last_position", m_player.getPosition());
}

void App::search(const std::string& query)
{
    m_appStatus.set("Searching...");
    auto lfmTracks = m_lastfm.trackSearch(query, 15);
    m_appStatus.clear();

    std::vector<TrackResult> results;
    results.reserve(lfmTracks.size());
    for (auto& t : lfmTracks) {
        std::string id = t.mbid;
        if (id.empty()) {
            id = std::to_string(std::hash<std::string>{}(t.artist + '\t' + t.name));
        }
        results.push_back({ std::move(id), std::move(t.artist), {}, std::move(t.name), {}, 0 });
    }

    {
        std::lock_guard lock(m_enrichMutex);
        m_currentResults = results;
        while (!m_enrichPending.empty()) { m_enrichPending.pop(); }
        while (!m_enrichDone.empty())    { m_enrichDone.pop(); }
        for (int i = 0; i < (int)results.size(); ++i) { m_enrichPending.push(i); }
    }

    std::lock_guard lock(m_searchMutex);
    m_pendingResults = std::move(results);

    startEnrichment();
}

void App::startEnrichment()
{
    if (m_enrichRunning.exchange(true)) { return; }

    std::thread([this]() {
        while (true) {
            int idx = -1;
            std::string artist, track;
            {
                std::lock_guard lock(m_enrichMutex);
                if (m_enrichPending.empty()) { break; }
                idx = m_enrichPending.front();
                m_enrichPending.pop();
                artist = m_currentResults[idx].artist;
                track  = m_currentResults[idx].track;
            }

            auto info = m_lastfm.trackInfo(artist, track);
            EnrichUpdate upd { idx, {}, {}, 0 };
            if (info) {
                upd.album    = info->album;
                upd.duration = info->duration > 3600 ? info->duration / 1000 : info->duration; // LastFM returns ms
                for (auto& img : info->images) {
                    if (img.size == "extralarge" && !img.url.empty()) {
                        upd.albumArtUrl = img.url;
                        break;
                    }
                }
                // fallback to large if extralarge missing
                if (upd.albumArtUrl.empty()) {
                    for (auto& img : info->images) {
                        if (img.size == "large" && !img.url.empty()) {
                            upd.albumArtUrl = img.url;
                            break;
                        }
                    }
                }
            }

            {
                std::lock_guard lock(m_enrichMutex);
                m_currentResults[idx].album       = upd.album;
                m_currentResults[idx].albumArtUrl = upd.albumArtUrl;
                m_currentResults[idx].duration    = upd.duration;
                m_enrichDone.push(upd);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
        m_enrichRunning = false;
    }).detach();
}

void App::flushSearch()
{
    {
        std::lock_guard lock(m_searchMutex);
        if (m_pendingResults.has_value())
        {
            m_mainView.showSearch(*m_pendingResults);
            m_pendingResults.reset();
            return;
        }
    }

    // apply any enrichment updates that arrived since the last frame
    std::vector<TrackResult> snapshot;
    {
        std::lock_guard eLock(m_enrichMutex);
        if (m_enrichDone.empty()) { return; }
        while (!m_enrichDone.empty()) { m_enrichDone.pop(); }
        snapshot = m_currentResults;
    }
    m_mainView.showSearch(snapshot);
}

void App::tick()
{
    m_player.tick();
    m_mediaManager.processCompletions();
    m_playQueue.tick();
    m_radio.tick();
    flushSearch();
}

void App::draw()
{
    m_topBar.draw();
    m_bottomBar.draw();
    m_sideBar.draw();
    m_mainView.draw();
    m_settingsModal.draw();
}
