#pragma once

#include <functional>
#include <vector>

#include "core/Track.hpp"

class SearchResults
{
public:
    using ClickHandler = std::function<void(int index, const std::vector<TrackResult>& all)>;

    void setResults(const std::vector<TrackResult>& results);
    void setClickHandler(ClickHandler handler) { m_onClick = std::move(handler); }
    void draw();

private:
    void drawRow(int index, const TrackResult& result);

    std::vector<TrackResult> m_results;
    ClickHandler m_onClick;
};
