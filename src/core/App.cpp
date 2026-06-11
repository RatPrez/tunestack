#include "core/App.hpp"
#include <SDL3/SDL.h>

App::App(SDL_Renderer* renderer)
{
    EventBus::m_instance = &m_eventBus;

    m_eventBus.on("open_settings", [this]() {
        m_settingsModal.open();
    });
}

void App::draw()
{
    m_player.tick();
    m_mediaManager.processCompletions();

    m_topBar.draw();
    m_bottomBar.draw();
    m_sideBar.draw();
    m_mainView.draw();
    m_settingsModal.draw();
}
