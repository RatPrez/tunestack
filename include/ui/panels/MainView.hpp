#pragma once

#include <vector>

#include "core/Track.hpp"
#include "ui/panels/SearchResults.hpp"

enum class MainViewState { Search, LikedSongs };

class MainView
{
public:
    MainView();
    ~MainView();

    void draw();
    void showSearch(const std::vector<TrackResult>& results);
    void showLikedSongs();

private:
    void drawSearch();

    MainViewState m_state = MainViewState::LikedSongs;
    SearchResults m_searchResults;
};
