#include "core/App.hpp"

#include <filesystem>
#include <thread>

#include <SDL3/SDL.h>

App::App(SDL_Renderer* renderer)
    : m_lastfm(m_settings.get<std::string>("lastfm_api_key"))
    , m_playlists(m_settings.configDir())
    , m_radio(m_player, m_mediaManager, m_lastfm)
{
    EventBus::m_instance = &m_eventBus;

    m_eventBus.on<std::string>("search", [this](const std::string& query) {
        std::thread([this, query]() { search(query); }).detach();
    });

    m_eventBus.on("open_settings", [this]() {
        m_settingsModal.open();
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
    auto lfmTracks = m_lastfm.trackSearch(query, 25);
    m_appStatus.clear();

    std::vector<TrackResult> results;
    results.reserve(lfmTracks.size());
    for (auto& t : lfmTracks) {
        std::string id = t.mbid;
        if (id.empty()) {
            id = std::to_string(std::hash<std::string>{}(t.artist + '\t' + t.name));
        }
        results.push_back({ std::move(id), std::move(t.artist), std::move(t.album), std::move(t.name) });
    }

    std::lock_guard lock(m_searchMutex);
    m_pendingResults = std::move(results);
}

void App::flushSearch()
{
    std::lock_guard lock(m_searchMutex);
    if (m_pendingResults.has_value())
    {
        m_mainView.showSearch(*m_pendingResults);
        m_pendingResults.reset();
    }
}

void App::tick()
{
    m_player.tick();
    m_mediaManager.processCompletions();
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
