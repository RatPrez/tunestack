#pragma once

class SettingsModal
{
public:
    void open();
    void draw();

private:
    bool m_open = false;
    char m_apiKeyBuf[256] = {};
    char m_lastfmKeyBuf[256] = {};
};
