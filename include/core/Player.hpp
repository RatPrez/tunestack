#pragma once

#include <memory>
#include <string>
#include <vector>

struct PlayerImpl;

class Player
{
public:
    Player();
    ~Player();

    void load(const std::string& filePath);
    void play();
    void pause();
    void next();
    void prev();
    void tick();
    void toggleShuffle();
    void toggleRepeat();

    void setVolume(float volume);
    float getVolume() const;

    void setPosition(float position);
    float getPosition() const;

    void setMuted(bool state);

    bool isPlaying() const;
    bool isShuffle() const;
    bool isRepeat() const;
    bool isMuted() const;

    const std::string& getArtist() const;
    const std::string& getTrack() const;
    const std::string& getArtworkKey() const;
    const std::string& getArtworkBytes() const;
    std::string getTimeElapsed() const;
    std::string getTimeTotal() const;

    static Player* Instance() { return m_instance; }

private:
    void startAudio(const std::string& filePath);

    std::unique_ptr<PlayerImpl> m_impl;
    float m_volume = 0.7f;
    bool m_shuffle = false;
    bool m_repeat = false;
    bool m_wasPlaying = false;
    bool m_muted = false;
    std::string m_artist;
    std::string m_track;
    std::string m_artworkKey;
    std::string m_artworkBytes;
    std::vector<std::string> m_history;
    int m_historyIndex = -1;

    static Player* m_instance;
};
