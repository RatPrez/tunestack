#include "core/Window.hpp"

#include <exception>

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <IconsFontAwesome5.h>

#include "core/App.hpp"
#include "ui/Fonts.hpp"

static constexpr int kFpsTarget = 60;
static constexpr float kFpsMs = 1000.f / kFpsTarget;

Window* Window::m_instance = nullptr;

Window::Window()
{
    m_instance = this;
    if (!initSDL() || !initImGui()) {
        m_running = false;
    }
}

Window::~Window()
{
    // ImGUI Destroy
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // SDL Destroy
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Window::run()
{
    if (!m_running) return;

    App app(m_renderer);

    while (m_running) {
        uint64_t frameStart = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch(event.type) {
            case SDL_EVENT_QUIT:
                m_running = false;
                break;
            }
        }

        app.tick();

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        app.draw();

        ImGui::Render();
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
        SDL_RenderPresent(m_renderer);

        const uint64_t frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < static_cast<uint32_t>(kFpsMs)) {
            SDL_Delay(static_cast<uint32_t>(kFpsMs) - static_cast<uint32_t>(frameTime));
        }
    }
}

bool Window::initSDL()
{
    try {
        SDL_Init(SDL_INIT_VIDEO);
        if (!SDL_CreateWindowAndRenderer("Tunestack", 1280, 720, SDL_WINDOW_RESIZABLE, &m_window, &m_renderer)) {
            SDL_Log( "SDL could not initialize! SDL error: %s\n", SDL_GetError() );
            return false;
        }
        SDL_SetWindowMinimumSize(m_window, 800, 500);
    } catch (std::exception e) {

        return false;
    }

    return true;
}

bool Window::initImGui()
{
    try {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();

        loadFonts();

        ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
        ImGui_ImplSDLRenderer3_Init(m_renderer);
    } catch (std::exception e) {

        return false;
    }

    return true;
}

void Window::loadFonts()
{
    const auto& io = ImGui::GetIO();

    static const ImWchar k_iconRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    auto mergeIcons = [&](float size) {
        ImFontConfig cfg;
        cfg.MergeMode = true;
        cfg.PixelSnapH = true;
        cfg.GlyphMinAdvanceX = size * 2.0f / 3.0f;
        io.Fonts->AddFontFromFileTTF("assets/fonts/" FONT_ICON_FILE_NAME_FAS, size * 2.0f / 3.0f, &cfg, k_iconRanges);
    };

    // first font becomes default, so we load regular first.
    Fonts::regular = io.Fonts->AddFontFromFileTTF("assets/fonts/VT323-Regular.ttf", 16.f);
    mergeIcons(20.0f); // merge font awesome icons into default font

    Fonts::large = io.Fonts->AddFontFromFileTTF("assets/fonts/VT323-Regular.ttf", 20.f);
    mergeIcons(24.0f);

    Fonts::small = io.Fonts->AddFontFromFileTTF("assets/fonts/VT323-Regular.ttf", 12.f);
}
