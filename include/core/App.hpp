#pragma once

#include "core/EventBus.hpp"
#include "core/Player.hpp"
#include "core/MediaManager.hpp"
#include "core/Settings.hpp"

// panels
#include "ui/panels/TopBar.hpp"
#include "ui/panels/BottomBar.hpp"
#include "ui/panels/SideBar.hpp"
#include "ui/panels/MainView.hpp"
#include "ui/panels/SettingsModal.hpp"

// forward declare because, why not?
class SDL_Renderer;

class App
{
public:
    explicit App(SDL_Renderer* renderer);
    void draw();

private:
    EventBus m_eventBus;
    Settings m_settings;
    Player m_player;
    MediaManager m_mediaManager;

    TopBar m_topBar;
    BottomBar m_bottomBar;
    SideBar m_sideBar;
    MainView m_mainView;
    SettingsModal m_settingsModal;

};
