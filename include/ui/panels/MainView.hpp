#pragma once

#include <vector>

#include "core/Track.hpp"
#include "ui/panels/SearchResults.hpp"

enum class MainViewState { Home, Search };

class MainView
{
public:
    MainView();
    ~MainView();

    void draw();
    void showSearch(const std::vector<TrackResult>& results);

private:
    void drawHome();
    void drawSearch();

    MainViewState m_state = MainViewState::Home;
    SearchResults m_searchResults;
};
