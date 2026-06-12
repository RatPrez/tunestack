#include "core/Radio.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <random>
#include <string>
#include <thread>

#include "core/AppStatus.hpp"
#include "core/MediaManager.hpp"
#include "core/PlayQueue.hpp"
#include "core/Player.hpp"
#include "core/Track.hpp"


Radio::Radio(Player& player, MediaManager& mediaManager, LastFM& lastfm)
    : m_player(player), m_mediaManager(mediaManager), m_lastfm(lastfm)
{}

void Radio::tick()
{
    if (!m_player.isPlaying() || m_player.hasNext()) { return; }
    if (PlayQueue::Instance() && !PlayQueue::Instance()->isEmpty()) { return; }

    const std::string currentTrack  = m_player.getTrack();
    const std::string currentArtist = m_player.getArtist();
    if (currentTrack.empty() || currentTrack == m_sourceTrack) { return; }
    if (m_fetching.exchange(true)) { return; }

    m_sourceTrack = currentTrack;

    std::thread([this, artist = currentArtist, track = currentTrack]() {
        AppStatus::Instance()->set("Finding similar tracks...");
        auto similar = m_lastfm.trackSimilar(artist, track, 15);
        AppStatus::Instance()->clear();

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

            TrackResult result { id, pick.artist, pick.album, pick.name, {}, 0 };

            // fetch art URL via track.getInfo — we're already on a background thread
            if (auto info = m_lastfm.trackInfo(pick.artist, pick.name)) {
                if (!info->album.empty()) { result.album = info->album; }
                for (auto& img : info->images) {
                    if (img.size == "extralarge" && !img.url.empty()) { result.albumArtUrl = img.url; break; }
                }
                if (result.albumArtUrl.empty()) {
                    for (auto& img : info->images) {
                        if (img.size == "large" && !img.url.empty()) { result.albumArtUrl = img.url; break; }
                    }
                }
            }

            m_mediaManager.requestTrack(result, [this, result](TrackStatus status, const std::string& path) {
                if (status == TrackStatus::Ready) {
                    Player::Instance()->queue(path);
                } else if (status == TrackStatus::RateLimited) {
                    AppStatus::Instance()->set("YouTube rate limited — retrying in 60s...");
                    std::thread([this, result]() {
                        std::this_thread::sleep_for(std::chrono::seconds(60));
                        AppStatus::Instance()->clear();
                        // only retry if the user hasn't queued something else in the meantime
                        if (Player::Instance()->hasNext() || !PlayQueue::Instance()->isEmpty()) { return; }
                        m_mediaManager.requestTrack(result, [](TrackStatus s, const std::string& p) {
                            if (s == TrackStatus::Ready && !Player::Instance()->hasNext()) {
                                Player::Instance()->queue(p);
                            }
                        });
                    }).detach();
                }
            });
        }

        m_fetching = false;
    }).detach();
}
