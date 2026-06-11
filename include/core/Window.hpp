#pragma once

class SDL_Window;
class SDL_Renderer;

class Window
{
public:
    Window();
    ~Window();
    void run();

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    bool m_running = true;

    bool initSDL();
    bool initImGui();

    void loadFonts();

};
