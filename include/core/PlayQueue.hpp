#pragma once

#include <atomic>
#include <deque>
#include <vector>

#include "core/Track.hpp"

class MediaManager;
class Player;

class PlayQueue
{
public:
    PlayQueue(Player& player, MediaManager& mediaManager);

    // replace queue with tracks[startIndex+1..end], immediately request tracks[startIndex]
    void play(const std::vector<TrackResult>& tracks, int startIndex);
    void clear();
    bool isEmpty() const { return m_queue.empty() && !m_requesting; }

    void tick();

    static PlayQueue* Instance() { return m_instance; }

private:
    void requestNext();

    Player&       m_player;
    MediaManager& m_mediaManager;
    std::deque<TrackResult> m_queue;
    std::atomic<bool> m_requesting { false };

    static PlayQueue* m_instance;
};
