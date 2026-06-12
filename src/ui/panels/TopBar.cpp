#include "ui/panels/TopBar.hpp"

#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "core/AppStatus.hpp"
#include "core/EventBus.hpp"
#include "ui/Colors.hpp"
#include "ui/Flags.hpp"
#include "ui/Layout.hpp"
#include "ui/Fonts.hpp"

TopBar::TopBar()
{

}

TopBar::~TopBar()
{

}

void TopBar::draw()
{
    auto& io = ImGui::GetIO();

    const float searchX = (io.DisplaySize.x / 2.0f) - (Layout::kSearchBarWidth / 2.0f);
    const float sideWidth = searchX - (Layout::kBarPadding * 2);
    const ImVec2 sideSize { sideWidth, Layout::kTopBarHeight };
    const ImVec2 searchSize { Layout::kSearchBarWidth, Layout::kTopBarHeight };

    drawLeft(
        ImVec2(Layout::kBarPadding, Layout::kBarPadding),
        sideSize
    );

    drawSearch(
        ImVec2(searchX, Layout::kBarPadding),
        searchSize
    );

    drawRight(
        ImVec2(searchX + Layout::kSearchBarWidth + Layout::kBarPadding, Layout::kBarPadding),
        sideSize
    );
}

void TopBar::drawLeft(const ImVec2& windowPos, const ImVec2& windowSize)
{
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kTransparent);
    ImGui::Begin("tbl", nullptr, Flags::kStaticDiv);


    // drawButton(0, Anchor::TopLeft, ICON_FA_HOME, "Home");


    ImGui::End();
    ImGui::PopStyleColor(2);
}

void TopBar::drawRight(const ImVec2& windowPos, const ImVec2& windowSize)
{
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kTransparent);
    ImGui::Begin("tbr", nullptr, Flags::kStaticDiv);

    if (drawButton(0, Anchor::TopRight, ICON_FA_COG, "Settings")) {
        EventBus::Instance()->emit("open_settings");
    }

    auto* status = AppStatus::Instance();
    if (status && status->isActive()) {
        static constexpr const char* kSpinner[] = { "|", "/", "-", "\\" };
        const int frame = static_cast<int>(ImGui::GetTime() * 8.0) % 4;

        const std::string msg = std::string(kSpinner[frame]) + "  " + status->get();
        const ImVec2 textSize = ImGui::CalcTextSize(msg.c_str());
        const float cogW = Layout::kTopBarHeight;
        const float x = ImGui::GetWindowWidth() - cogW - textSize.x - Layout::kBarPadding * 2;
        const float y = (Layout::kTopBarHeight - textSize.y) / 2.0f;

        ImGui::SetCursorPos({ x, y });
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::kWhiteHalf);
        ImGui::TextUnformatted(msg.c_str());
        ImGui::PopStyleColor();
    }

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

    // search
    const float inputHeight = ImGui::GetFrameHeight();
    ImGui::SetCursorPos({ (float)Layout::kBarPadding, (ImGui::GetWindowHeight() - inputHeight) / 2.5f});
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - (Layout::kBarPadding * 2) - Layout::kTopBarHeight);

    ImGui::PushFont(Fonts::large);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, Colors::kTransparent);

    bool submitted = ImGui::InputTextWithHint(
        "##search", "What do you want to play?",
        m_searchBuf, sizeof(m_searchBuf),
        ImGuiInputTextFlags_EnterReturnsTrue
    );
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    // icon
    const bool clicked = drawButton(0, Anchor::TopRight, ICON_FA_SEARCH, "Search");
    if (clicked) { submitted = true; }

    if (submitted && m_searchBuf[0] != '\0') {
        EventBus::Instance()->emit<std::string>("search", std::string(m_searchBuf));
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
}

bool TopBar::drawButton(const int& index, const Anchor& anchor, const std::string& label, const std::string& hint)
{
    const float buttonSize = Layout::kTopBarHeight;
    ImVec2 pos {0, 0};

    switch (anchor) {
    case Anchor::TopLeft:
        pos = { (float)(index * (buttonSize)) + 1, 1 };
        break;
    case Anchor::TopRight:
        pos = { ImGui::GetWindowWidth() - (float)((index + 1) * (buttonSize)) + 1, 1 };
        break;
    }

    ImGui::SetCursorPos(pos);

    bool hovered = ImGui::IsMouseHoveringRect(
        ImVec2(ImGui::GetWindowPos().x + pos.x, ImGui::GetWindowPos().y + pos.y),
        ImVec2(ImGui::GetWindowPos().x + pos.x + buttonSize, ImGui::GetWindowPos().y + pos.y + buttonSize)
    );

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_Text, hovered ? Colors::kWhite : Colors::kWhiteHalf);
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::kTransparent);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, Colors::kTransparent);
    ImGui::PushFont(Fonts::large);

    bool clicked = ImGui::Button(label.c_str(), { buttonSize - 2, buttonSize - 2 });

    ImGui::PopFont();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(5);

    if (hint != "") {
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", hint.c_str());
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
    }


    return clicked;
}
