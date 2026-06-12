#include "ui/panels/SearchResults.hpp"

#include <format>
#include <string>
#include <imgui.h>
#include <IconsFontAwesome5.h>

#include "core/MediaManager.hpp"
#include "core/PlaylistManager.hpp"
#include "core/Player.hpp"
#include "core/TextureCache.hpp"
#include "ui/Colors.hpp"

constexpr float kRowHeight  = 56.0f;
constexpr float kArtSize    = 40.0f;
constexpr float kArtColW    = 52.0f; // art + padding

static std::string formatDuration(int seconds)
{
    if (seconds <= 0) { return {}; }
    return std::format("{}:{:02d}", seconds / 60, seconds % 60);
}

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

    if (!ImGui::BeginTable("##results", 5, kTableFlags, ImGui::GetContentRegionAvail())) { return; }

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("#",        ImGuiTableColumnFlags_WidthFixed,   kArtColW);
    ImGui::TableSetupColumn("Title",    ImGuiTableColumnFlags_WidthStretch, 0.4f);
    ImGui::TableSetupColumn("Artist",   ImGuiTableColumnFlags_WidthStretch, 0.3f);
    ImGui::TableSetupColumn("Album",    ImGuiTableColumnFlags_WidthStretch, 0.3f);
    ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed,   58.0f);

    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    for (int col = 0; col < 5; ++col)
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

    auto* player        = Player::Instance();
    const bool isActive = player->getTrackId() == result.id && player->isPlaying();
    bool rowHovered     = false;

    // col 1 first — selectable spans all cols, then title text
    ImGui::TableSetColumnIndex(1);
    {
        const float rowTop = ImGui::GetCursorPosY();
        const ImVec2 rowScreenMin = ImGui::GetCursorScreenPos();

        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 1, 1, 0.05f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(1, 1, 1, 0.08f));
        const bool clicked = ImGui::Selectable(
            "##row", false,
            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap,
            ImVec2(0, kRowHeight)
        );
        ImGui::PopStyleColor(2);

        rowHovered = ImGui::IsMouseHoveringRect(rowScreenMin, { rowScreenMin.x + ImGui::GetWindowWidth(), rowScreenMin.y + kRowHeight });
        if (rowHovered) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }

        // title — vertically centered, accent colour if active
        ImGui::SameLine();
        ImGui::SetCursorPosY(rowTop + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
        if (isActive) ImGui::PushStyleColor(ImGuiCol_Text, Colors::kAccent);
        ImGui::TextUnformatted(result.track.c_str());
        if (isActive) ImGui::PopStyleColor();

        // like button — visible on hover
        if (rowHovered) {
            auto* pm = PlaylistManager::Instance();
            const bool liked = pm->hasTrack(PlaylistManager::kLikedSongs, result.id);
            ImGui::SameLine();
            ImGui::SetCursorPosY(rowTop + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, liked ? Colors::kTermainlGreen : Colors::kWhiteHalf);
            ImGui::PushStyleColor(ImGuiCol_Button,        Colors::kTransparent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::kTransparent);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Colors::kTransparent);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            const char* likeIcon = liked ? ICON_FA_CHECK : ICON_FA_PLUS;
            if (ImGui::SmallButton(likeIcon)) {
                if (liked) pm->removeTrack(PlaylistManager::kLikedSongs, result.id);
                else       pm->addTrack(PlaylistManager::kLikedSongs, result);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(liked ? "Remove from Liked Songs" : "Add to Liked Songs");
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
        }

        if (MediaManager::Instance()->isDownloaded(result.id))
        {
            ImGui::SameLine();
            ImGui::SetCursorPosY(rowTop + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::kWhiteHalf);
            ImGui::TextUnformatted(ICON_FA_FILE_ARCHIVE);
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Cached"); }
        }

        if (clicked) {
            if (isActive) {
                player->pause();
            } else if (m_onClick) {
                m_onClick(index, m_results);
            }
        }
    }

    // col 0 — artwork thumbnail (if loaded), with play/pause overlay on hover
    ImGui::TableSetColumnIndex(0);
    {
        const ImVec2 cellPos   = ImGui::GetCursorScreenPos();
        const float  cellCentX = cellPos.x + (kArtColW - kArtSize) / 2.0f;
        const float  cellCentY = cellPos.y + (kRowHeight - kArtSize) / 2.0f;
        const ImVec2 artMin    = { cellCentX, cellCentY };
        const ImVec2 artMax    = { cellCentX + kArtSize, cellCentY + kArtSize };

        auto* dl = ImGui::GetWindowDrawList();

        SDL_Texture* tex = result.albumArtUrl.empty()
            ? nullptr
            : TextureCache::Instance()->getUrl(result.albumArtUrl);

        if (tex) {
            dl->AddImage((ImTextureID)(intptr_t)tex, artMin, artMax);
        }

        if (rowHovered || isActive) {
            // dim overlay so icon is readable over art
            if (tex) { dl->AddRectFilled(artMin, artMax, IM_COL32(0, 0, 0, 110)); }

            const char* icon = isActive ? ICON_FA_PAUSE : ICON_FA_PLAY;
            const ImVec2 iconSz = ImGui::CalcTextSize(icon);
            const ImVec2 iconPos = {
                artMin.x + (kArtSize - iconSz.x) / 2.0f,
                artMin.y + (kArtSize - iconSz.y) / 2.0f
            };
            dl->AddText(iconPos, IM_COL32(89, 166, 255, 255), icon);
        } else if (!tex) {
            // no art yet — show track number
            const std::string num = std::to_string(index + 1);
            const ImVec2 numSz = ImGui::CalcTextSize(num.c_str());
            dl->AddText(
                { cellPos.x + (kArtColW - numSz.x) / 2.0f,
                  cellPos.y + (kRowHeight - numSz.y) / 2.0f },
                IM_COL32(128, 128, 128, 200), num.c_str()
            );
        }

        // advance cursor past the cell so ImGui doesn't collapse it
        ImGui::Dummy({ kArtColW, kRowHeight });
    }

    // col 2 — artist
    ImGui::TableSetColumnIndex(2);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, isActive ? Colors::kAccentDim : Colors::kWhiteHalf);
    ImGui::TextUnformatted(result.artist.c_str());
    ImGui::PopStyleColor();

    // col 3 — album
    ImGui::TableSetColumnIndex(3);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, isActive ? Colors::kAccentDim : Colors::kWhiteHalf);
    ImGui::TextUnformatted(result.album.c_str());
    ImGui::PopStyleColor();

    // col 4 — duration
    ImGui::TableSetColumnIndex(4);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (kRowHeight - ImGui::GetTextLineHeight()) / 2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::kWhiteHalf);
    ImGui::TextUnformatted(formatDuration(result.duration).c_str());
    ImGui::PopStyleColor();

    ImGui::PopID();
}
