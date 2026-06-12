#pragma once

#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/Track.hpp"
#include "api/YouTube.hpp"

enum class TrackStatus { Ready, Downloading, NotFound, RateLimited, Error };

using TrackCallback = std::function<void(TrackStatus, const std::string&)>;

class MediaManager
{
public:
    explicit MediaManager(const std::filesystem::path& libraryPath);

    void requestTrack(const TrackResult& track, TrackCallback onComplete);
    void processCompletions();
    bool isDownloaded(const std::string& id) const;
    std::string getPath(const std::string& id) const;
    std::uintmax_t cacheSize() const;
    void clearCache();

    static MediaManager* Instance() { return m_instance; }

private:
    static MediaManager* m_instance;

    struct CompletionEvent
    {
        TrackCallback callback;
        TrackStatus status;
        std::string filePath;
    };

    bool trackExists(const std::string& id) const;
    std::string trackPath(const std::string& id) const;
    void fetchAndDownload(const TrackResult& track);
    void writeMetadata(const std::string& filePath, const TrackResult& track, const std::string& artworkBytes) const;
    void pushCompletions(const std::string& id, TrackStatus status, const std::string& filePath);

    std::string m_libraryPath;
    YouTube m_youtube;
    std::mutex m_downloadMutex;
    std::unordered_map<std::string, std::vector<TrackCallback>> m_pendingCallbacks;
    std::mutex m_queueMutex;
    std::queue<CompletionEvent> m_completionQueue;
};
