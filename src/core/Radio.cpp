#include "core/Radio.hpp"

#include <algorithm>
#include <functional>
#include <random>
#include <string>
#include <thread>

#include "core/MediaManager.hpp"
#include "core/Player.hpp"
#include "core/Track.hpp"


Radio::Radio(Player& player, MediaManager& mediaManager, LastFM& lastfm)
    : m_player(player), m_mediaManager(mediaManager), m_lastfm(lastfm)
{}

void Radio::tick()
{
    if (!m_player.isPlaying() || m_player.hasNext()) { return; }

    const std::string currentTrack  = m_player.getTrack();
    const std::string currentArtist = m_player.getArtist();
    if (currentTrack.empty() || currentTrack == m_sourceTrack) { return; }
    if (m_fetching.exchange(true)) { return; }

    m_sourceTrack = currentTrack;

    std::thread([this, artist = currentArtist, track = currentTrack]() {
        auto similar = m_lastfm.trackSimilar(artist, track, 15);

        // build candidates — filter out anything already in history
        std::vector<LastFMTrack> candidates;
        for (auto& t : similar) {
            std::string id = t.mbid;
            if (id.empty()) {
                id = std::to_string(std::hash<std::string>{}(t.artist + '\t' + t.name));
            }
            if (!m_player.hasPath(m_mediaManager.getPath(id))) {
                candidates.push_back(std::move(t));
            }
        }

        if (!candidates.empty()) {
            std::mt19937 rng(std::random_device{}());
            auto& pick = candidates[std::uniform_int_distribution<size_t>(0, candidates.size() - 1)(rng)];

            std::string id = pick.mbid;
            if (id.empty()) {
                id = std::to_string(std::hash<std::string>{}(pick.artist + '\t' + pick.name));
            }

            TrackResult result { std::move(id), std::move(pick.artist), std::move(pick.album), std::move(pick.name) };
            m_mediaManager.requestTrack(result, [](TrackStatus status, const std::string& path) {
                if (status == TrackStatus::Ready) { Player::Instance()->queue(path); }
            });
        }

        m_fetching = false;
    }).detach();
}
