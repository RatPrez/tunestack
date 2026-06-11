#include "core/MediaManager.hpp"

MediaManager* MediaManager::m_instance = nullptr;

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#include <mpegfile.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <textidentificationframe.h>

#include <filesystem>
#include <format>
#include <thread>

namespace fs = std::filesystem;

MediaManager::MediaManager(const std::string& libraryPath)
    : m_libraryPath(libraryPath)
{
    m_instance = this;
    fs::create_directories(libraryPath);
}

void MediaManager::requestTrack(const ITunesResult& track, TrackCallback onComplete)
{
    if (trackExists(track.trackId)) {
        std::lock_guard lock(m_queueMutex);
        m_completionQueue.push({ std::move(onComplete), TrackStatus::Ready, trackPath(track.trackId) });
        return;
    }

    {
        std::lock_guard lock(m_downloadMutex);
        if (m_pendingCallbacks.count(track.trackId)) {
            m_pendingCallbacks[track.trackId].push_back(std::move(onComplete));
            return;
        }
        m_pendingCallbacks[track.trackId].push_back(std::move(onComplete));
    }

    std::thread([this, track]() { fetchAndDownload(track); }).detach();
}

void MediaManager::processCompletions()
{
    std::queue<CompletionEvent> local;
    {
        std::lock_guard lock(m_queueMutex);
        std::swap(local, m_completionQueue);
    }
    while (!local.empty()) {
        auto& e = local.front();
        e.callback(e.status, e.filePath);
        local.pop();
    }
}

bool MediaManager::trackExists(const int trackId) const
{
    return fs::exists(trackPath(trackId));
}

bool MediaManager::isDownloaded(int trackId) const
{
    return trackExists(trackId);
}

std::string MediaManager::trackPath(const int trackId) const
{
    return std::format("{}{}.mp3", m_libraryPath, trackId);
}

std::string MediaManager::highResArtworkUrl(const std::string& url) const
{
    std::string result = url;
    if (auto pos = result.find("100x100bb"); pos != std::string::npos) {
        result.replace(pos, 9, "600x600bb");
    }
    return result;
}

void MediaManager::fetchAndDownload(const ITunesResult& track)
{
    auto ytResult = m_youtube.search(std::format("{} - {} (Audio)", track.track, track.artist));
    if (!ytResult) {
        pushCompletions(track.trackId, TrackStatus::NotFound, "");
        return;
    }

    const std::string path = trackPath(track.trackId);
    if (!m_youtube.download(*ytResult, path)) {
        pushCompletions(track.trackId, TrackStatus::Error, "");
        return;
    }

    std::string artworkBytes;
    const std::string artUrl = highResArtworkUrl(track.artworkUrl);
    const size_t schemeEnd = artUrl.find("://");
    if (schemeEnd != std::string::npos) {
        const std::string hostAndPath = artUrl.substr(schemeEnd + 3);
        const size_t pathStart = hostAndPath.find('/');
        if (pathStart != std::string::npos) {
            httplib::Client client("https://" + hostAndPath.substr(0, pathStart));
            client.set_follow_location(true);
            if (auto res = client.Get(hostAndPath.substr(pathStart)); res && res->status == 200) {
                artworkBytes = std::move(res->body);
            }
        }
    }

    writeMetadata(path, track, artworkBytes);
    pushCompletions(track.trackId, TrackStatus::Ready, path);
}

void MediaManager::writeMetadata(
    const std::string& filePath, const ITunesResult& track, const std::string& artworkBytes) const
{
    TagLib::MPEG::File file(filePath.c_str());
    TagLib::ID3v2::Tag* tag = file.ID3v2Tag(true);

    tag->setTitle(TagLib::String(track.track, TagLib::String::UTF8));
    tag->setArtist(TagLib::String(track.artist, TagLib::String::UTF8));
    tag->setAlbum(TagLib::String(track.collection, TagLib::String::UTF8));

    auto* txxx = new TagLib::ID3v2::UserTextIdentificationFrame(TagLib::String::UTF8);
    txxx->setDescription("TUNESTASH_TRACK_ID");
    txxx->setText(TagLib::String(std::to_string(track.trackId), TagLib::String::UTF8));
    tag->addFrame(txxx);

    if (!artworkBytes.empty()) {
        auto* apic = new TagLib::ID3v2::AttachedPictureFrame();
        apic->setMimeType("image/jpeg");
        apic->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
        apic->setPicture(TagLib::ByteVector(artworkBytes.data(), static_cast<unsigned int>(artworkBytes.size())));
        tag->addFrame(apic);
    }

    file.save();
}

void MediaManager::pushCompletions(const int trackId, const TrackStatus status, const std::string& filePath)
{
    std::vector<TrackCallback> callbacks;
    {
        std::lock_guard lock(m_downloadMutex);
        auto it = m_pendingCallbacks.find(trackId);
        if (it != m_pendingCallbacks.end()) {
            callbacks = std::move(it->second);
            m_pendingCallbacks.erase(it);
        }
    }
    std::lock_guard lock(m_queueMutex);
    for (auto& cb : callbacks) {
        m_completionQueue.push({ std::move(cb), status, filePath });
    }
}
