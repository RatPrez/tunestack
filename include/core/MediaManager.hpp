#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "api/ITunes.hpp"
#include "api/YouTube.hpp"

enum class TrackStatus { Ready, Downloading, NotFound, Error };

using TrackCallback = std::function<void(TrackStatus, const std::string&)>;

class MediaManager
{
public:
    explicit MediaManager(const std::string& libraryPath = "./downloads/");

    void requestTrack(const ITunesResult& track, TrackCallback onComplete);
    void processCompletions();

private:
    struct CompletionEvent
    {
        TrackCallback callback;
        TrackStatus status;
        std::string filePath;
    };

    bool trackExists(int trackId) const;
    std::string trackPath(int trackId) const;
    std::string highResArtworkUrl(const std::string& url) const;
    void fetchAndDownload(const ITunesResult& track);
    void writeMetadata(const std::string& filePath, const ITunesResult& track, const std::string& artworkBytes) const;
    void pushCompletions(int trackId, TrackStatus status, const std::string& filePath);

    std::string m_libraryPath;
    YouTube m_youtube;
    std::mutex m_downloadMutex;
    std::unordered_map<int, std::vector<TrackCallback>> m_pendingCallbacks;
    std::mutex m_queueMutex;
    std::queue<CompletionEvent> m_completionQueue;
};
