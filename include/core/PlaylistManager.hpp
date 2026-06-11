#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/Track.hpp"

class PlaylistManager
{
public:
    static constexpr const char* kLikedSongs = "liked_songs";

    explicit PlaylistManager(const std::filesystem::path& configDir);

    std::vector<std::string>    getPlaylists() const;
    void                        createPlaylist(const std::string& name);

    void                        addTrack(const std::string& playlist, const TrackResult& track);
    void                        removeTrack(const std::string& playlist, const std::string& trackId);
    bool                        hasTrack(const std::string& playlist, const std::string& trackId) const;
    const std::vector<TrackResult>& getTracks(const std::string& playlist) const;

    static PlaylistManager* Instance() { return m_instance; }

private:
    void load();
    void save(const std::string& name) const;
    void saveIndex() const;
    std::filesystem::path playlistPath(const std::string& name) const;

    std::filesystem::path m_configDir;
    std::unordered_map<std::string, std::vector<TrackResult>> m_playlists;

    static PlaylistManager* m_instance;
};
