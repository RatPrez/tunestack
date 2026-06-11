#include "ui/panels/BottomBar.hpp"

#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "core/Player.hpp"
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
    auto* player = Player::Instance();
    ImGui::SetCursorPos({ 0, 0 });

    const std::string& artworkKey = player->getArtworkKey();
    SDL_Texture* artwork = nullptr;
    if (!artworkKey.empty()) {
        auto* cache = TextureCache::Instance();
        artwork = cache->get(artworkKey);
        if (!artwork) {
            const std::string& bytes = player->getArtworkBytes();
            cache->addImageBytes(artworkKey, bytes.data(), (int)bytes.size());
        }
    }

    if (artwork) {
        ImGui::Image((ImTextureID)(intptr_t)artwork, ImVec2(Layout::kSideBarWidth, Layout::kSideBarWidth));
    } else {
        ImGui::Dummy(ImVec2(Layout::kSideBarWidth, Layout::kSideBarWidth));
    }

    ImGui::SameLine(0.f, 16.f);
    ImGui::SetCursorPosY((Layout::kSideBarWidth - ImGui::GetTextLineHeightWithSpacing() * 2.f) * .5f);
    ImGui::BeginGroup();

    const std::string& track = player->getTrack();
    ImGui::Text("%s", track.empty() ? "No Track" : track.c_str());

    ImGui::PushStyleColor(ImGuiCol_Text, Colors::kWhiteHalf);
    const std::string& artist = player->getArtist();
    ImGui::Text("%s", artist.empty() ? "" : artist.c_str());
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

    auto* player = Player::Instance();
    float progress = player->getPosition();

    // invisible slider first so we can query active state before drawing
    ImGui::SetCursorPos({ 0, (float)Layout::kBottomBarHeight - trackHeight / 2.0f - kSliderH / 2.0f });
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0, 0, 0, 0));
    ImGui::SetNextItemWidth(windowWidth);
    if (ImGui::SliderFloat("##progress", &progress, 0.0f, 1.0f, "")) {
        player->setPosition(progress);
    }
    ImGui::PopStyleColor(5);

    const float sliderY = y + (trackHeight - kSliderH) / 2.0f;
    const bool hovered = ImGui::IsMouseHoveringRect({ windowPos.x, sliderY }, { windowPos.x + windowWidth, sliderY + kSliderH });
    const bool active = ImGui::IsItemActive();

    if (hovered || active) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }

    const ImU32 trackColor = ImGui::ColorConvertFloat4ToU32(Colors::kTrackIdle);
    const ImU32 trackColorProg = ImGui::ColorConvertFloat4ToU32((hovered || active) ? Colors::kTrackProgressHover : Colors::kTrackProgress);

    float fillX = windowPos.x + (windowWidth * progress);

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

    auto* player = Player::Instance();

    // shuffle, back, play, forward, loop
    if (drawButton("shuffle", ICON_FA_RANDOM, player->isShuffle() ? "Shuffle: On" : "Shuffle: Off"))
        player->toggleShuffle();
    ImGui::SameLine();
    if (drawButton("prev", ICON_FA_STEP_BACKWARD, "Previous"))
        player->prev();
    ImGui::SameLine();
    if (drawButton("play", player->isPlaying() ? ICON_FA_PAUSE : ICON_FA_PLAY, player->isPlaying() ? "Pause" : "Play")) {
        if (player->isPlaying()) player->pause(); else player->play();
    }
    ImGui::SameLine();
    if (drawButton("next", ICON_FA_STEP_FORWARD, "Next"))
        player->next();
    ImGui::SameLine();
    if (drawButton("loop", ICON_FA_REDO, player->isRepeat() ? "Repeat: On" : "Repeat: Off"))
        player->toggleRepeat();
}

void BottomBar::drawVolume()
{
    constexpr float kVolumeTotalW  = kButtonSize + kVolumeSliderW;

    const float windowWidth = ImGui::GetWindowWidth();
    const float centerY = ((float)Layout::kSideBarWidth - kButtonSize) / 2.0f;
    const float startX  = windowWidth - kVolumeTotalW - Layout::kBarPadding;
    const float sliderX = startX + kButtonSize;
    const float sliderY = centerY + (kButtonSize - kVolumeSliderH) / 2.0f;

    auto* player = Player::Instance();

    // icon — toggles mute on click
    const bool muted = player->isMuted();
    const float volume = player->getVolume();
    const bool effectivelyMuted = muted || volume == 0.0f;
    const char* volumeIcon = effectivelyMuted ? ICON_FA_VOLUME_MUTE : ICON_FA_VOLUME_UP;
    const char* volumeHint = effectivelyMuted ? "Unmute" : "Mute";
    ImGui::SetCursorPos({ startX, centerY });
    if (drawButton("volume", volumeIcon, volumeHint)) {
        player->setMuted(!muted);
    }

    // invisible slider first so we can query active state before drawing
    float sliderVolume = volume;
    ImGui::SetCursorPos({ sliderX, sliderY });
    ImGui::PushStyleColor(ImGuiCol_FrameBg,          ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0, 0, 0, 0));
    ImGui::SetNextItemWidth(kVolumeSliderW);
    if (ImGui::SliderFloat("##volume", &sliderVolume, 0.0f, 1.0f, "")) {
        player->setVolume(sliderVolume);
    }
    ImGui::PopStyleColor(5);

    const bool active = ImGui::IsItemActive();

    ImGui::SetCursorPos({ sliderX, sliderY });
    const ImVec2 screenPos = ImGui::GetCursorScreenPos();
    const bool hovered = ImGui::IsMouseHoveringRect(screenPos, { screenPos.x + kVolumeSliderW, screenPos.y + kVolumeSliderH });

    if (hovered || active) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetTooltip("%d%%", (int)(sliderVolume * 100.0f));
    }

    const ImU32 trackColor = ImGui::ColorConvertFloat4ToU32(Colors::kTrackIdle);
    const ImU32 fillColor  = ImGui::ColorConvertFloat4ToU32((hovered || active) ? Colors::kTrackProgressHover : Colors::kTrackProgress);
    const float fillX = screenPos.x + (kVolumeSliderW * (effectivelyMuted ? 0.0f : sliderVolume));

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
