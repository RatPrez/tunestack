#pragma once

#include <cstdint>
#include <string>

class SettingsModal
{
public:
    void open();
    void draw();

private:
    bool m_open = false;
    char m_apiKeyBuf[256]     = {};
    char m_lastfmKeyBuf[256]  = {};

    std::uintmax_t m_cacheBytes = 0;
    bool           m_cacheCleared = false;

    static std::string formatBytes(std::uintmax_t bytes);
};
