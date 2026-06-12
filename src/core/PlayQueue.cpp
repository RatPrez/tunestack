#include "core/PlayQueue.hpp"

#include "core/MediaManager.hpp"
#include "core/Player.hpp"

PlayQueue* PlayQueue::m_instance = nullptr;

PlayQueue::PlayQueue(Player& player, MediaManager& mediaManager)
    : m_player(player), m_mediaManager(mediaManager)
{
    m_instance = this;
}

void PlayQueue::play(const std::vector<TrackResult>& tracks, int startIndex)
{
    clear();

    // queue everything after the clicked track
    for (int i = startIndex + 1; i < (int)tracks.size(); ++i) {
        m_queue.push_back(tracks[i]);
    }

    // immediately request the clicked track to play
    const TrackResult& first = tracks[startIndex];
    m_requesting = true;
    m_mediaManager.requestTrack(first, [](TrackStatus status, const std::string& path) {
        if (status == TrackStatus::Ready) { Player::Instance()->load(path); }
        PlayQueue::Instance()->m_requesting = false;
    });
}

void PlayQueue::clear()
{
    m_queue.clear();
    m_requesting = false;
}

void PlayQueue::tick()
{
    if (m_queue.empty() || m_requesting || m_player.hasNext()) { return; }
    requestNext();
}

void PlayQueue::requestNext()
{
    if (m_queue.empty()) { return; }
    TrackResult next = m_queue.front();
    m_queue.pop_front();

    m_requesting = true;
    m_mediaManager.requestTrack(next, [](TrackStatus status, const std::string& path) {
        if (status == TrackStatus::Ready) { Player::Instance()->queue(path); }
        PlayQueue::Instance()->m_requesting = false;
    });
}
