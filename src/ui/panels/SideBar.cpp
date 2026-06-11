#include "ui/panels/SideBar.hpp"

#include <imgui.h>

#include "Colors.hpp"
#include "Flags.hpp"
#include "Layout.hpp"

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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kPanelBorder);
    ImGui::Begin("sb", nullptr, Flags::kStaticDiv);


    itemButton(0);
    itemButton(1);
    itemButton(2);


    ImGui::End();
    ImGui::PopStyleColor(2);

}

bool SideBar::itemButton(const int& index)
{
    const float buttonSize = Layout::kSideBarWidth - (Layout::kBarPadding * 2);
    const ImVec2 windowPos = ImGui::GetWindowPos();

    ImGui::SetCursorPos({
        Layout::kBarPadding,
        Layout::kBarPadding + ((buttonSize + Layout::kBarPadding ) * index)
    });

    ImGui::PushStyleColor(ImGuiCol_ButtonActive, Colors::kTermainlGreen);
    ImGui::Button("Test", {buttonSize, buttonSize});
    ImGui::PopStyleColor();

    return false;
}
