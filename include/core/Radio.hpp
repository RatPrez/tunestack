#pragma once

#include <atomic>
#include <string>

#include "api/LastFM.hpp"

class MediaManager;
class Player;

class Radio
{
public:
    Radio(Player& player, MediaManager& mediaManager, LastFM& lastfm);

    void tick();

private:
    Player&       m_player;
    MediaManager& m_mediaManager;
    LastFM&       m_lastfm;

    std::atomic<bool> m_fetching { false };
    std::string       m_sourceTrack;
};
