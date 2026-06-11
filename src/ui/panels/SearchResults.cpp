#include "ui/panels/SearchResults.hpp"

#include <string>
#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "core/MediaManager.hpp"
#include "core/Player.hpp"
#include "ui/Colors.hpp"

constexpr float kRowHeight = 56.0f;

void SearchResults::setResults(const std::vector<TrackResult>& results)
{
    m_results = results;
}

void SearchResults::draw()
{
    constexpr ImGuiTableFlags kTableFlags =
        ImGuiTableFlags_SizingFixedFit  |
        ImGuiTableFlags_BordersInnerH   |
        ImGuiTableFlags_ScrollY         |
        ImGuiTableFlags_NoHostExtendX;

    if (!ImGui::BeginTable("##results", 4, kTableFlags, ImGui::GetContentRegionAvail())) { return; }

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("#",      ImGuiTableColumnFlags_WidthFixed,   40.0f);
    ImGui::TableSetupColumn("Title",  ImGuiTableColumnFlags_WidthStretch, 0.4f);
    ImGui::TableSetupColumn("Artist", ImGuiTableColumnFlags_WidthStretch, 0.3f);
    ImGui::TableSetupColumn("Album",  ImGuiTableColumnFlags_WidthStretch, 0.3f);

    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    for (int col = 0; col < 4; ++col)
    {
        ImGui::TableSetColumnIndex(col);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::kWhiteHalf);
        ImGui::TextUnformatted(ImGui::TableGetColumnName(col));
        ImGui::PopStyleColor();
    }

    for (int i = 0; i < (int)m_results.size(); ++i)
    {
        drawRow(i, m_results[i]);
    }

    ImGui::EndTable();
}

void SearchResults::drawRow(int index, const TrackResult& result)
{
    ImGui::PushID(index);
    ImGui::TableNextRow(0, kRowHeight);

    bool rowHovered = false;

    // col 1 first — selectable spans all cols, title text
    ImGui::TableSetColumnIndex(1);
    {
        const float rowTop = ImGui::GetCursorPosY();

        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 1, 1, 0.05f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(1, 1, 1, 0.08f));
        const bool clicked = ImGui::Selectable(
            "##row", false,
            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
            ImVec2(0, kRowHeight)
        );
        ImGui::PopStyleColor(2);

        rowHovered = ImGui::IsItemHovered();
        if (rowHovered) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }

        // title — vertically centered
        ImGui::SameLine();
        ImGui::SetCursorPosY(rowTop + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
        ImGui::TextUnformatted(result.track.c_str());

        if (MediaManager::Instance()->isDownloaded(result.id))
        {
            ImGui::SameLine();
            ImGui::SetCursorPosY(rowTop + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::kWhiteHalf);
            ImGui::TextUnformatted(ICON_FA_FILE_ARCHIVE);
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Cached"); }
        }

        if (clicked)
        {
            MediaManager::Instance()->requestTrack(result, [](TrackStatus status, const std::string& path) {
                if (status == TrackStatus::Ready) { Player::Instance()->load(path); }
            });
        }
    }

    // col 0 — # or play icon, centered
    ImGui::TableSetColumnIndex(0);
    {
        const char* label = rowHovered ? ICON_FA_PLAY : std::to_string(index + 1).c_str();
        const ImVec2 textSize = ImGui::CalcTextSize(label);
        const float colW = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPos({
            ImGui::GetCursorPosX() + (colW - textSize.x) / 2.0f,
            ImGui::GetCursorPosY() + (kRowHeight - textSize.y) / 2.0f
        });
        ImGui::PushStyleColor(ImGuiCol_Text, rowHovered ? Colors::kAccent : Colors::kWhiteHalf);
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
    }

    // col 2 — artist vertically centered
    ImGui::TableSetColumnIndex(2);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::kWhiteHalf);
    ImGui::TextUnformatted(result.artist.c_str());
    ImGui::PopStyleColor();

    // col 3 — album vertically centered
    ImGui::TableSetColumnIndex(3);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::kWhiteHalf);
    ImGui::TextUnformatted(result.album.c_str());
    ImGui::PopStyleColor();

    ImGui::PopID();
}
