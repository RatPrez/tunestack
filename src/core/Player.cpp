#include "core/Player.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <mpegfile.h>
#include <id3v2tag.h>
#include <frames/attachedpictureframe.h>

#include <cmath>
#include <format>

Player* Player::m_instance = nullptr;

struct PlayerImpl
{
    ma_engine engine;
    ma_sound sound;
    bool soundLoaded = false;
};

static std::string formatTime(float seconds)
{
    const int total = static_cast<int>(std::floor(seconds));
    return std::format("{:02d}:{:02d}", total / 60, total % 60);
}

Player::Player()
    : m_impl(std::make_unique<PlayerImpl>())
{
    m_instance = this;
    ma_engine_init(nullptr, &m_impl->engine);
}

Player::~Player()
{
    if (m_impl->soundLoaded) { ma_sound_uninit(&m_impl->sound); }
    ma_engine_uninit(&m_impl->engine);
}

void Player::startAudio(const std::string& filePath)
{
    if (m_impl->soundLoaded) {
        ma_sound_uninit(&m_impl->sound);
        m_impl->soundLoaded = false;
    }
    if (ma_sound_init_from_file(&m_impl->engine, filePath.c_str(), 0, nullptr, nullptr, &m_impl->sound) != MA_SUCCESS) { return; }

    m_impl->soundLoaded = true;
    ma_sound_set_volume(&m_impl->sound, m_volume);
    ma_sound_start(&m_impl->sound);
    m_wasPlaying = true;

    m_artist.clear();
    m_track.clear();
    m_artworkKey.clear();
    m_artworkBytes.clear();

    TagLib::MPEG::File tagFile(filePath.c_str());
    if (tagFile.isValid()) {
        if (auto* tag = tagFile.tag()) {
            m_artist = tag->artist().to8Bit(true);
            m_track = tag->title().to8Bit(true);
        }
        if (auto* id3 = tagFile.ID3v2Tag()) {
            auto frames = id3->frameListMap()["APIC"];
            if (!frames.isEmpty()) {
                auto* apic = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
                if (apic) {
                    const auto& pic = apic->picture();
                    m_artworkBytes.assign(pic.data(), pic.size());
                    m_artworkKey = filePath;
                }
            }
        }
    }
}

void Player::load(const std::string& filePath)
{
    if (m_historyIndex + 1 < static_cast<int>(m_history.size())) {
        m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());
    }
    m_history.push_back(filePath);
    m_historyIndex = static_cast<int>(m_history.size()) - 1;
    startAudio(filePath);
}

void Player::queue(const std::string& filePath)
{
    // drop any forward history beyond current position, then append without playing
    if (m_historyIndex + 1 < static_cast<int>(m_history.size())) {
        m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());
    }
    m_history.push_back(filePath);
}

void Player::play()
{
    if (!m_impl->soundLoaded) { return; }
    ma_sound_start(&m_impl->sound);
    m_wasPlaying = true;
}

void Player::pause()
{
    if (!m_impl->soundLoaded) { return; }
    ma_sound_stop(&m_impl->sound);
    m_wasPlaying = false;
}

void Player::next()
{
    if (m_historyIndex < static_cast<int>(m_history.size()) - 1) {
        startAudio(m_history[++m_historyIndex]);
    }
}

void Player::prev()
{
    if (m_historyIndex > 0) {
        startAudio(m_history[--m_historyIndex]);
    }
}

void Player::tick()
{
    if (!m_impl->soundLoaded || !m_wasPlaying) { return; }
    if (ma_sound_is_playing(&m_impl->sound)) { return; }
    if (m_repeat) {
        ma_sound_seek_to_pcm_frame(&m_impl->sound, 0);
        ma_sound_start(&m_impl->sound);
    } else {
        next();
    }
}

void Player::toggleShuffle() { m_shuffle = !m_shuffle; }
void Player::toggleRepeat() { m_repeat = !m_repeat; }

void Player::setVolume(const float volume)
{
    if (volume > 0 && m_muted) {
        m_muted = false;
    }
    m_volume = volume;
    if (m_impl->soundLoaded) { ma_sound_set_volume(&m_impl->sound, volume); }
}

float Player::getVolume() const { return m_muted ? 0.f : m_volume; }

void Player::setMuted(bool state)
{
    m_muted = state;
}

void Player::setPosition(const float position)
{
    if (!m_impl->soundLoaded) { return; }
    ma_uint64 length;
    ma_sound_get_length_in_pcm_frames(&m_impl->sound, &length);
    ma_sound_seek_to_pcm_frame(&m_impl->sound, static_cast<ma_uint64>(position * static_cast<float>(length)));
}

float Player::getPosition() const
{
    if (!m_impl->soundLoaded) { return 0.0f; }
    ma_uint64 cursor, length;
    ma_sound_get_cursor_in_pcm_frames(&m_impl->sound, &cursor);
    ma_sound_get_length_in_pcm_frames(&m_impl->sound, &length);
    if (length == 0) { return 0.0f; }
    return static_cast<float>(cursor) / static_cast<float>(length);
}

bool Player::isPlaying() const
{
    if (!m_impl->soundLoaded) { return false; }
    return ma_sound_is_playing(&m_impl->sound);
}

bool Player::hasNext() const { return m_historyIndex < static_cast<int>(m_history.size()) - 1; }
bool Player::hasPath(const std::string& filePath) const
{
    for (const auto& p : m_history) { if (p == filePath) { return true; } }
    return false;
}
bool Player::isShuffle() const { return m_shuffle; }
bool Player::isRepeat() const { return m_repeat; }
bool Player::isMuted() const { return m_muted; }

const std::string& Player::getArtist() const { return m_artist; }
const std::string& Player::getTrack() const { return m_track; }
const std::string& Player::getArtworkKey() const { return m_artworkKey; }
const std::string& Player::getArtworkBytes() const { return m_artworkBytes; }

std::string Player::getTimeElapsed() const
{
    if (!m_impl->soundLoaded) { return "00:00"; }
    ma_uint64 cursor;
    ma_sound_get_cursor_in_pcm_frames(&m_impl->sound, &cursor);
    return formatTime(static_cast<float>(cursor) / static_cast<float>(ma_engine_get_sample_rate(&m_impl->engine)));
}

std::string Player::getTimeTotal() const
{
    if (!m_impl->soundLoaded) { return "00:00"; }
    ma_uint64 length;
    ma_sound_get_length_in_pcm_frames(&m_impl->sound, &length);
    return formatTime(static_cast<float>(length) / static_cast<float>(ma_engine_get_sample_rate(&m_impl->engine)));
}

float Player::getDuration() const
{
    if (!m_impl->soundLoaded) { return 0.0f; }
    ma_uint64 length;
    ma_sound_get_length_in_pcm_frames(&m_impl->sound, &length);
    return static_cast<float>(length) / static_cast<float>(ma_engine_get_sample_rate(&m_impl->engine));
}

const std::string& Player::getCurrentFilePath() const
{
    static const std::string kEmpty;
    if (m_historyIndex < 0 || m_historyIndex >= static_cast<int>(m_history.size())) { return kEmpty; }
    return m_history[m_historyIndex];
}
