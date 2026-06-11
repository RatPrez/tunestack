#pragma once

#include <cstdint>
#include <vector>
#include <string>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

struct ITunesResult
{
    int64_t trackId = 0;
    int64_t artistId = 0;
    int trackNumber = 0;
    std::string artist;
    std::string collection;
    std::string track;
    std::string artworkUrl;
};

struct ITunesAlbum
{
    int64_t albumId = 0;
    int64_t artistId = 0;
    int trackCount = 0;
    // releaseDate ?? what type?
    std::vector<ITunesResult> tracks;
};

class ITunes
{
public:
    ITunes();

    // fetch artwork URL for a given artist+track (used internally by MediaManager)
    std::string fetchArtwork(const std::string& artist, const std::string& track);

    ITunesAlbum getAlbum(int64_t albumId);
    std::vector<ITunesResult> getTop25Songs(const std::string& region = "us");

private:
    std::string formatTerm(const std::string& query);
    httplib::Client m_httpClient;
    httplib::Client m_httpClient2;
};
