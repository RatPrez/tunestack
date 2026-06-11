#include "ui/panels/SideBar.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>

#include "ui/Colors.hpp"
#include "ui/Flags.hpp"
#include "ui/Layout.hpp"

#include "core/TextureCache.hpp"

SideBar::SideBar()
{

}

SideBar::~SideBar()
{

}

void SideBar::draw()
{
    auto& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(
        ImVec2(Layout::kBarPadding, Layout::kBarPadding + ( Layout::kTopBarHeight + Layout::kBarPadding ))
    );
    ImGui::SetNextWindowSize(
        ImVec2(
            Layout::kSideBarWidth,
            io.DisplaySize.y - Layout::kBottomBarHeight - Layout::kTopBarHeight - (Layout::kBarPadding * 4)
        )
    );

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kTransparent);
    ImGui::Begin("sb", nullptr, Flags::kStaticDiv);


    itemButton(0, "Liked Songs");
    itemButton(1, "Playlist 1");
    itemButton(2, "Playlist 2");


    ImGui::End();
    ImGui::PopStyleColor(2);

}

bool SideBar::itemButton(const int& index, const std::string& label)
{
    const float buttonSize = Layout::kSideBarWidth;
    const ImVec2 windowPos = ImGui::GetWindowPos();

    ImGui::SetCursorPos({0, ((buttonSize + Layout::kBarPadding ) * index) });
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

    auto* artwork = TextureCache::Instance()->get("assets/images/placeholder.jpg");
    if (artwork) {
        ImGui::ImageButton(label.c_str(), (ImTextureID)(intptr_t)artwork, {buttonSize, buttonSize});
    } else {
        ImGui::Button(label.c_str(), {buttonSize, buttonSize});
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", label.c_str());
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    return false;
}
