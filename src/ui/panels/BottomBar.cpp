#include "ui/panels/BottomBar.hpp"

#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "core/TextureCache.hpp"

#include "ui/Colors.hpp"
#include "ui/Flags.hpp"
#include "ui/Layout.hpp"
#include "ui/Fonts.hpp"

constexpr int kButtonSize = 40;
constexpr float kVolumeSliderW = 100.0f;
constexpr float kVolumeSliderH = 10.0f;

BottomBar::BottomBar()
{

}

BottomBar::~BottomBar()
{

}

void BottomBar::draw()
{
    auto& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(
        ImVec2(Layout::kBarPadding, io.DisplaySize.y - Layout::kBottomBarHeight - Layout::kBarPadding)
    );
    ImGui::SetNextWindowSize(
        ImVec2(io.DisplaySize.x - (Layout::kBarPadding * 2), Layout::kBottomBarHeight)
    );

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kTransparent);
    ImGui::Begin("bb", nullptr, Flags::kStaticDiv);

    drawTrackInfo();
    drawTrackProgress();
    drawControls();
    drawVolume();

    ImGui::End();
    ImGui::PopStyleColor(2);

}

void BottomBar::drawTrackInfo()
{
    ImGui::SetCursorPos({ 0, 0 });

    SDL_Texture* artwork = TextureCache::Instance()->get("assets/images/placeholder.jpg");
    if (artwork) {
        ImGui::Image((ImTextureID)(intptr_t)artwork, ImVec2(Layout::kSideBarWidth, Layout::kSideBarWidth));
    } else {
        ImGui::Dummy(ImVec2(Layout::kSideBarWidth, Layout::kSideBarWidth));
    }

    ImGui::SameLine(0.f, 16.f);
    ImGui::SetCursorPosY((Layout::kSideBarWidth - ImGui::GetTextLineHeightWithSpacing() * 2.f) * .5f);
    ImGui::BeginGroup();

    ImGui::Text("Track Name");

    ImGui::PushStyleColor(ImGuiCol_Text, Colors::kWhiteHalf);
    ImGui::Text("Artist Name");
    ImGui::PopStyleColor();

    ImGui::EndGroup();
}

void BottomBar::drawTrackProgress()
{
    const float trackHeight = Layout::kBottomBarHeight - Layout::kSideBarWidth;

    const ImVec2 windowPos = ImGui::GetWindowPos();
    const float windowWidth = ImGui::GetWindowWidth();
    const float y = windowPos.y + Layout::kBottomBarHeight - trackHeight;

    constexpr float kSliderH = 16.0f;

    // invisible slider first so we can query active state before drawing
    ImGui::SetCursorPos({ 0, (float)Layout::kBottomBarHeight - trackHeight / 2.0f - kSliderH / 2.0f });
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0, 0, 0, 0));
    ImGui::SetNextItemWidth(windowWidth);
    ImGui::SliderFloat("##progress", &m_progress, 0.0f, 1.0f, "");
    ImGui::PopStyleColor(5);

    const float sliderY = y + (trackHeight - kSliderH) / 2.0f;
    const bool hovered = ImGui::IsMouseHoveringRect({ windowPos.x, sliderY }, { windowPos.x + windowWidth, sliderY + kSliderH });
    const bool active = ImGui::IsItemActive();

    if (hovered || active) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }

    const ImU32 trackColor = ImGui::ColorConvertFloat4ToU32(Colors::kTrackIdle);
    const ImU32 trackColorProg = ImGui::ColorConvertFloat4ToU32((hovered || active) ? Colors::kTrackProgressHover : Colors::kTrackProgress);

    float fillX = windowPos.x + (windowWidth * m_progress);

    auto* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled({ windowPos.x, y }, { windowPos.x + windowWidth, y + trackHeight }, trackColor);
    dl->AddRectFilled({ windowPos.x, y }, { fillX, y + trackHeight }, trackColorProg);
}

void BottomBar::drawControls()
{
    const float btnsWidth = kButtonSize * 5.0f + ImGui::GetStyle().ItemSpacing.x * 4.0f;

    ImGui::SetCursorPos(ImVec2(
        (ImGui::GetWindowWidth() - btnsWidth) / 2.0f,
        ((float)Layout::kSideBarWidth - kButtonSize) / 4.0f
    ));

    // shuffle, back, play, forward, loop
    drawButton("shuffle", ICON_FA_RANDOM, "Shuffle");
    ImGui::SameLine();
    drawButton("prev", ICON_FA_STEP_BACKWARD, "Previous");
    ImGui::SameLine();
    drawButton("play", ICON_FA_PLAY, "Play/Pause");
    ImGui::SameLine();
    drawButton("next", ICON_FA_STEP_FORWARD, "Next");
    ImGui::SameLine();
    drawButton("loop", ICON_FA_REDO, "Loop");
}

void BottomBar::drawVolume()
{
    constexpr float kVolumeTotalW  = kButtonSize + kVolumeSliderW;

    const float windowWidth = ImGui::GetWindowWidth();
    const float centerY = ((float)Layout::kSideBarWidth - kButtonSize) / 2.0f;
    const float startX  = windowWidth - kVolumeTotalW - Layout::kBarPadding;
    const float sliderX = startX + kButtonSize;
    const float sliderY = centerY + (kButtonSize - kVolumeSliderH) / 2.0f;

    // icon — toggles mute on click
    const bool effectivelyMuted = m_muted || m_volume == 0.0f;
    const char* volumeIcon = effectivelyMuted ? ICON_FA_VOLUME_MUTE : ICON_FA_VOLUME_UP;
    const char* volumeHint = effectivelyMuted ? "Unmute" : "Mute";
    ImGui::SetCursorPos({ startX, centerY });
    if (drawButton("volume", volumeIcon, volumeHint)) {
        if (m_muted) {
            m_muted = false;
        } else {
            m_muted = true;
            if (m_volume == 0.0f) { m_volume = 0.75f; }
        }
    }

    // invisible slider first so we can query active state before drawing
    ImGui::SetCursorPos({ sliderX, sliderY });
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0, 0, 0, 0));
    ImGui::SetNextItemWidth(kVolumeSliderW);
    ImGui::SliderFloat("##volume", &m_volume, 0.0f, 1.0f, "");
    ImGui::PopStyleColor(5);

    const bool active = ImGui::IsItemActive();

    ImGui::SetCursorPos({ sliderX, sliderY });
    const ImVec2 screenPos = ImGui::GetCursorScreenPos();
    const bool hovered = ImGui::IsMouseHoveringRect(screenPos, { screenPos.x + kVolumeSliderW, screenPos.y + kVolumeSliderH });

    if (hovered || active) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetTooltip("%d%%", (int)(m_volume * 100.0f));
    }

    const ImU32 trackColor = ImGui::ColorConvertFloat4ToU32(Colors::kTrackIdle);
    const ImU32 fillColor  = ImGui::ColorConvertFloat4ToU32((hovered || active) ? Colors::kTrackProgressHover : Colors::kTrackProgress);
    const float fillX = screenPos.x + (kVolumeSliderW * (effectivelyMuted ? 0.0f : m_volume));

    auto* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(screenPos, { screenPos.x + kVolumeSliderW, screenPos.y + kVolumeSliderH }, trackColor);
    dl->AddRectFilled(screenPos, { fillX, screenPos.y + kVolumeSliderH }, fillColor);
}

bool BottomBar::drawButton(const char* id, const char* icon, const char* hint)
{
    ImGui::PushID(id);

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    bool hovered = ImGui::IsMouseHoveringRect(pos, { pos.x + kButtonSize, pos.y + kButtonSize });

    if (hovered) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetTooltip("%s", hint);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_Text, hovered ? Colors::kWhite : Colors::kWhiteHalf);
    ImGui::PushFont(Fonts::large);

    const bool clicked = ImGui::Button(icon, { kButtonSize, kButtonSize });

    ImGui::PopFont();
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    ImGui::PopID();
    return clicked;
}
