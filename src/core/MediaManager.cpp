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
#include <functional>
#include <thread>

namespace fs = std::filesystem;

// stable filename from a LastFM id (mbid or artist+track hash)
static std::string safeId(const std::string& id)
{
    // mbids are already filename-safe (hex + hyphens); others get hashed
    bool safe = true;
    for (char c : id) {
        if (!std::isalnum((unsigned char)c) && c != '-' && c != '_') { safe = false; break; }
    }
    if (safe && !id.empty()) { return id; }
    return std::to_string(std::hash<std::string>{}(id));
}

MediaManager::MediaManager(const std::string& libraryPath)
    : m_libraryPath(libraryPath)
{
    m_instance = this;
    fs::create_directories(libraryPath);
}

void MediaManager::requestTrack(const TrackResult& track, TrackCallback onComplete)
{
    if (trackExists(track.id)) {
        std::lock_guard lock(m_queueMutex);
        m_completionQueue.push({ std::move(onComplete), TrackStatus::Ready, trackPath(track.id) });
        return;
    }

    {
        std::lock_guard lock(m_downloadMutex);
        if (m_pendingCallbacks.count(track.id)) {
            m_pendingCallbacks[track.id].push_back(std::move(onComplete));
            return;
        }
        m_pendingCallbacks[track.id].push_back(std::move(onComplete));
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

bool MediaManager::trackExists(const std::string& id) const
{
    return fs::exists(trackPath(id));
}

bool MediaManager::isDownloaded(const std::string& id) const
{
    return trackExists(id);
}

std::string MediaManager::getPath(const std::string& id) const
{
    return trackPath(id);
}

std::string MediaManager::trackPath(const std::string& id) const
{
    return m_libraryPath + safeId(id) + ".mp3";
}

void MediaManager::fetchAndDownload(const TrackResult& track)
{
    auto ytResult = m_youtube.search(std::format("{} - {} (Audio)", track.track, track.artist));
    if (!ytResult) {
        pushCompletions(track.id, TrackStatus::NotFound, "");
        return;
    }

    const std::string path = trackPath(track.id);
    if (!m_youtube.download(*ytResult, path)) {
        pushCompletions(track.id, TrackStatus::Error, "");
        return;
    }

    // fetch artwork via iTunes
    std::string artworkBytes;
    const std::string artUrl = m_itunes.fetchArtwork(track.artist, track.track);
    if (!artUrl.empty()) {
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
    }

    writeMetadata(path, track, artworkBytes);
    pushCompletions(track.id, TrackStatus::Ready, path);
}

void MediaManager::writeMetadata(
    const std::string& filePath, const TrackResult& track, const std::string& artworkBytes) const
{
    TagLib::MPEG::File file(filePath.c_str());
    TagLib::ID3v2::Tag* tag = file.ID3v2Tag(true);

    tag->setTitle(TagLib::String(track.track, TagLib::String::UTF8));
    tag->setArtist(TagLib::String(track.artist, TagLib::String::UTF8));
    tag->setAlbum(TagLib::String(track.album, TagLib::String::UTF8));

    auto* txxx = new TagLib::ID3v2::UserTextIdentificationFrame(TagLib::String::UTF8);
    txxx->setDescription("TUNESTACK_TRACK_ID");
    txxx->setText(TagLib::String(track.id, TagLib::String::UTF8));
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

void MediaManager::pushCompletions(const std::string& id, const TrackStatus status, const std::string& filePath)
{
    std::vector<TrackCallback> callbacks;
    {
        std::lock_guard lock(m_downloadMutex);
        auto it = m_pendingCallbacks.find(id);
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
