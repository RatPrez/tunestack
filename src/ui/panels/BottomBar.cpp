#include "ui/panels/BottomBar.hpp"

#include <imgui.h>

#include "Colors.hpp"
#include "Flags.hpp"
#include "Layout.hpp"

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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kPanelBorder);
    ImGui::Begin("bb", nullptr, Flags::kStaticDiv);

    ImGui::End();
    ImGui::PopStyleColor(2);

}
