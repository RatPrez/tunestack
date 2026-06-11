#pragma once

#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "core/AppStatus.hpp"
#include "core/EventBus.hpp"
#include "core/PlaylistManager.hpp"
#include "core/Player.hpp"
#include "core/MediaManager.hpp"
#include "core/Radio.hpp"
#include "core/Settings.hpp"
#include "core/Track.hpp"
#include "api/LastFM.hpp"

// panels
#include "ui/panels/TopBar.hpp"
#include "ui/panels/BottomBar.hpp"
#include "ui/panels/SideBar.hpp"
#include "ui/panels/MainView.hpp"
#include "ui/panels/SettingsModal.hpp"

class SDL_Renderer;

class App
{
public:
    explicit App(SDL_Renderer* renderer);
    ~App();
    void tick();
    void draw();

private:
    AppStatus       m_appStatus;
    EventBus        m_eventBus;
    Settings        m_settings;
    PlaylistManager m_playlists;
    Player       m_player;
    MediaManager m_mediaManager;
    LastFM       m_lastfm;
    Radio        m_radio;

    TopBar        m_topBar;
    BottomBar     m_bottomBar;
    SideBar       m_sideBar;
    MainView      m_mainView;
    SettingsModal m_settingsModal;

    void search(const std::string& query);
    void flushSearch();

    std::mutex m_searchMutex;
    std::optional<std::vector<TrackResult>> m_pendingResults;
};
