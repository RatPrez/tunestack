#include "ui/panels/SideBar.hpp"

#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "core/EventBus.hpp"
#include "ui/Colors.hpp"
#include "ui/Flags.hpp"
#include "ui/Fonts.hpp"
#include "ui/Layout.hpp"

SideBar::SideBar() {}
SideBar::~SideBar() {}

void SideBar::draw()
{
    auto& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(
        ImVec2(Layout::kBarPadding, Layout::kBarPadding + Layout::kTopBarHeight + Layout::kBarPadding)
    );
    ImGui::SetNextWindowSize(ImVec2(
        Layout::kSideBarWidth,
        io.DisplaySize.y - Layout::kBottomBarHeight - Layout::kTopBarHeight - (Layout::kBarPadding * 4)
    ));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_Border,   Colors::kTransparent);
    ImGui::Begin("sb", nullptr, Flags::kStaticDiv);

    if (iconButton(0, ICON_FA_HEART, "Liked Songs")) {
        EventBus::Instance()->emit("show_liked_songs");
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
}

bool SideBar::iconButton(int index, const char* icon, const char* tooltip)
{
    const float size = Layout::kSideBarWidth;
    ImGui::SetCursorPos({ 0, (size + Layout::kBarPadding) * index });

    const ImVec2 screenPos = ImGui::GetCursorScreenPos();
    const bool hovered = ImGui::IsMouseHoveringRect(screenPos, { screenPos.x + size, screenPos.y + size });

    // gradient background + border drawn before the button
    {
        const ImVec2 pMin = screenPos;
        const ImVec2 pMax = { screenPos.x + size, screenPos.y + size };
        constexpr ImU32 kTop    = IM_COL32( 89, 166, 255,  35);  // accent blue
        constexpr ImU32 kBottom = IM_COL32( 46, 204, 150,  35);  // teal green
        constexpr ImU32 kBorder = IM_COL32(255, 255, 255,  76);  // kPanelBorder ~0.3 alpha
        auto* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilledMultiColor(pMin, pMax, kTop, kTop, kBottom, kBottom);
        dl->AddRect(pMin, pMax, kBorder);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,    { 0.f, 0.f });
    ImGui::PushStyleColor(ImGuiCol_Button,        Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_Text, hovered ? Colors::kWhite : Colors::kWhiteHalf);
    ImGui::PushFont(Fonts::large);

    const bool clicked = ImGui::Button(icon, { size, size });

    ImGui::PopFont();
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    if (hovered) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetTooltip("%s", tooltip);
    }

    return clicked;
}
