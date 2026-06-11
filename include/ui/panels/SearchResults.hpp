#pragma once

#include <vector>

#include "core/Track.hpp"

class SearchResults
{
public:
    void setResults(const std::vector<TrackResult>& results);
    void draw();

private:
    void drawRow(int index, const TrackResult& result);

    std::vector<TrackResult> m_results;
};
