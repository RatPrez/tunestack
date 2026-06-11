#include "core/App.hpp"
#include <SDL3/SDL.h>

App::App(SDL_Renderer* renderer)
{

}

void App::draw()
{
    m_player.tick();

    m_topBar.draw();
    m_bottomBar.draw();
    m_sideBar.draw();
    m_mainView.draw();
}
