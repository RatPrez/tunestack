#include "ui/panels/MainView.hpp"

#include <imgui.h>

#include "core/MediaManager.hpp"
#include "core/PlaylistManager.hpp"
#include "core/PlayQueue.hpp"
#include "core/Player.hpp"
#include "core/Track.hpp"
#include "ui/Colors.hpp"
#include "ui/Flags.hpp"
#include "ui/Layout.hpp"

static void playOnlyClicked(int index, const std::vector<TrackResult>& all)
{
    MediaManager::Instance()->requestTrack(all[index], [](TrackStatus status, const std::string& path) {
        if (status == TrackStatus::Ready) { Player::Instance()->load(path); }
    });
}

static void playFromPlaylist(int index, const std::vector<TrackResult>& all)
{
    PlayQueue::Instance()->play(all, index);
}

MainView::MainView() {}
MainView::~MainView() {}

void MainView::showSearch(const std::vector<TrackResult>& results)
{
    m_searchResults.setResults(results);
    m_searchResults.setClickHandler(playOnlyClicked);
    m_state = MainViewState::Search;
}

void MainView::showLikedSongs()
{
    m_searchResults.setResults(PlaylistManager::Instance()->getTracks(PlaylistManager::kLikedSongs));
    m_searchResults.setClickHandler(playFromPlaylist);
    m_state = MainViewState::LikedSongs;
}

void MainView::draw()
{
    auto& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(
        Layout::kSideBarWidth + (Layout::kBarPadding * 2),
        Layout::kTopBarHeight + (Layout::kBarPadding * 2)
    ));
    ImGui::SetNextWindowSize(ImVec2(
        io.DisplaySize.x - Layout::kSideBarWidth - (Layout::kBarPadding * 3),
        io.DisplaySize.y - Layout::kTopBarHeight - Layout::kBottomBarHeight - (Layout::kBarPadding * 4)
    ));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Colors::kPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border,   Colors::kPanelBorder);
    ImGui::Begin("mv", nullptr, Flags::kStaticDiv);

    switch (m_state)
    {
    case MainViewState::Search:     drawSearch(); break;
    case MainViewState::LikedSongs:
        // refresh list every frame so like/unlike is reflected immediately
        m_searchResults.setResults(PlaylistManager::Instance()->getTracks(PlaylistManager::kLikedSongs));
        m_searchResults.setClickHandler(playFromPlaylist);
        drawSearch();
        break;
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
}

void MainView::drawSearch() { m_searchResults.draw(); }
