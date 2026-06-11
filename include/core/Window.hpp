#pragma once

class SDL_Window;
class SDL_Renderer;

class Window
{
public:
    Window();
    ~Window();
    void run();

    static Window* Instance() { return m_instance; }

    SDL_Window* getWindow() { return m_window; }
    SDL_Renderer* getRenderer() { return m_renderer; }

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    static Window* m_instance;

    bool m_running = true;

    bool initSDL();
    bool initImGui();

    void loadFonts();

};
