#include "ui/panels/TopBar.hpp"

#include <imgui.h>

#include "Colors.hpp"
#include "Flags.hpp"
#include "Layout.hpp"

TopBar::TopBar()
{

}

TopBar::~TopBar()
{

}

void TopBar::draw()
{
    auto& io = ImGui::GetIO();

    const float thirds = (io.DisplaySize.x - (Layout::kBarPadding * 4)) / 3.0f;
    const ImVec2 windowSize { thirds, Layout::kTopBarHeight };

    drawLeft(
        ImVec2(Layout::kBarPadding, Layout::kBarPadding),
        windowSize
    );

    drawRight(
        ImVec2(
            (Layout::kBarPadding * 2) + thirds,
            Layout::kBarPadding
        ),
        windowSize
    );

    drawSearch(
        ImVec2(
            (Layout::kBarPadding * 3) + (thirds * 2),
            Layout::kBarPadding
        ),
        windowSize
    );
}

void TopBar::drawLeft(const ImVec2& windowPos, const ImVec2& windowSize)
{
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kPanelBorder);
    ImGui::Begin("tbl", nullptr, Flags::kStaticDiv);



    ImGui::End();
    ImGui::PopStyleColor(2);
}

void TopBar::drawRight(const ImVec2& windowPos, const ImVec2& windowSize)
{
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kPanelBorder);
    ImGui::Begin("tbr", nullptr, Flags::kStaticDiv);



    ImGui::End();
    ImGui::PopStyleColor(2);
}

void TopBar::drawSearch(const ImVec2& windowPos, const ImVec2& windowSize)
{
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kPanelBorder);
    ImGui::Begin("tbs", nullptr, Flags::kStaticDiv);



    ImGui::End();
    ImGui::PopStyleColor(2);
}
