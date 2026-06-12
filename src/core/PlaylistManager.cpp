#include "core/PlaylistManager.hpp"

#include <fstream>

#include <json.hpp>

using json = nlohmann::json;

PlaylistManager* PlaylistManager::m_instance = nullptr;

static const std::vector<TrackResult> kEmpty;

PlaylistManager::PlaylistManager(const std::filesystem::path& configDir)
    : m_configDir(configDir)
{
    m_instance = this;
    std::filesystem::create_directories(configDir);
    load();

    // ensure liked songs playlist always exists
    if (!m_playlists.count(kLikedSongs)) {
        createPlaylist(kLikedSongs);
    }
}

std::filesystem::path PlaylistManager::playlistPath(const std::string& name) const
{
    return m_configDir / ("playlist_" + name + ".json");
}

void PlaylistManager::load()
{
    const auto indexPath = m_configDir / "playlists.json";
    if (!std::filesystem::exists(indexPath)) { return; }

    std::ifstream f(indexPath);
    json index;
    try { index = json::parse(f); } catch (...) { return; }

    for (const auto& name : index) {
        const std::string n = name.get<std::string>();
        const auto path = playlistPath(n);
        if (!std::filesystem::exists(path)) { m_playlists[n] = {}; continue; }

        std::ifstream pf(path);
        json data;
        try { data = json::parse(pf); } catch (...) { m_playlists[n] = {}; continue; }

        auto& tracks = m_playlists[n];
        for (const auto& t : data) {
            tracks.push_back({
                t.value("id",          std::string{}),
                t.value("artist",      std::string{}),
                t.value("album",       std::string{}),
                t.value("track",       std::string{}),
                t.value("albumArtUrl", std::string{}),
                t.value("duration",    0),
            });
        }
    }
}

void PlaylistManager::saveIndex() const
{
    json index = json::array();
    for (const auto& [name, _] : m_playlists) { index.push_back(name); }

    std::ofstream f(m_configDir / "playlists.json");
    f << index.dump(2);
}

void PlaylistManager::save(const std::string& name) const
{
    const auto it = m_playlists.find(name);
    if (it == m_playlists.end()) { return; }

    json data = json::array();
    for (const auto& t : it->second) {
        data.push_back({
            {"id",          t.id},
            {"artist",      t.artist},
            {"album",       t.album},
            {"track",       t.track},
            {"albumArtUrl", t.albumArtUrl},
            {"duration",    t.duration},
        });
    }

    std::ofstream f(playlistPath(name));
    f << data.dump(2);
}

std::vector<std::string> PlaylistManager::getPlaylists() const
{
    std::vector<std::string> result;
    result.reserve(m_playlists.size());
    for (const auto& [name, _] : m_playlists) { result.push_back(name); }
    return result;
}

void PlaylistManager::createPlaylist(const std::string& name)
{
    if (m_playlists.count(name)) { return; }
    m_playlists[name] = {};
    saveIndex();
    save(name);
}

void PlaylistManager::addTrack(const std::string& playlist, const TrackResult& track)
{
    auto& tracks = m_playlists[playlist];
    for (const auto& t : tracks) { if (t.id == track.id) { return; } }
    tracks.push_back(track);
    save(playlist);
}

void PlaylistManager::removeTrack(const std::string& playlist, const std::string& trackId)
{
    auto it = m_playlists.find(playlist);
    if (it == m_playlists.end()) { return; }
    auto& tracks = it->second;
    tracks.erase(std::remove_if(tracks.begin(), tracks.end(),
        [&](const TrackResult& t) { return t.id == trackId; }), tracks.end());
    save(playlist);
}

bool PlaylistManager::hasTrack(const std::string& playlist, const std::string& trackId) const
{
    const auto it = m_playlists.find(playlist);
    if (it == m_playlists.end()) { return false; }
    for (const auto& t : it->second) { if (t.id == trackId) { return true; } }
    return false;
}

const std::vector<TrackResult>& PlaylistManager::getTracks(const std::string& playlist) const
{
    const auto it = m_playlists.find(playlist);
    if (it == m_playlists.end()) { return kEmpty; }
    return it->second;
}
