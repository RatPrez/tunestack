#include "ui/panels/MainView.hpp"

#include <imgui.h>

#include "Colors.hpp"
#include "Flags.hpp"
#include "Layout.hpp"

MainView::MainView()
{

}

MainView::~MainView()
{

}

void MainView::draw()
{
    auto& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(
        ImVec2(
            Layout::kSideBarWidth + (Layout::kBarPadding * 2),
            Layout::kTopBarHeight + (Layout::kBarPadding * 2)
        )
    );
    ImGui::SetNextWindowSize(
        ImVec2(
            io.DisplaySize.x - Layout::kSideBarWidth - (Layout::kBarPadding * 3),
            io.DisplaySize.y - Layout::kTopBarHeight - Layout::kBottomBarHeight - (Layout::kBarPadding * 4)
        )
    );

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kPanelBorder);
    ImGui::Begin("mv", nullptr, Flags::kStaticDiv);

    ImGui::End();
    ImGui::PopStyleColor(2);

}
