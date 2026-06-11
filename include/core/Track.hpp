#pragma once

#include <string>

struct TrackResult
{
    std::string id;       // LastFM mbid, or hash of artist+track if mbid empty
    std::string artist;
    std::string album;
    std::string track;
};
