#pragma once

// panels
#include "ui/panels/TopBar.hpp"
#include "ui/panels/BottomBar.hpp"
#include "ui/panels/SideBar.hpp"
#include "ui/panels/MainView.hpp"

// forward declare because, why not?
class SDL_Renderer;

class App
{
public:
    explicit App(SDL_Renderer* renderer);
    void draw();

private:
    TopBar m_topBar;
    BottomBar m_bottomBar;
    SideBar m_sideBar;
    MainView m_mainView;

};
