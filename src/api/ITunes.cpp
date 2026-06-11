#include "api/ITunes.hpp"

#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

ITunes::ITunes()
    : m_httpClient("https://itunes.apple.com")
    , m_httpClient2("https://rss.applemarketingtools.com")
{
    m_httpClient2.set_follow_location(true);
}

std::string ITunes::fetchArtwork(const std::string& artist, const std::string& track)
{
    try {
        const std::string term = formatTerm(artist + " " + track);
        auto res = m_httpClient.Get("/search?term=" + term + "&media=music&limit=1");
        if (!res || res->status != 200) { return {}; }
        auto body = json::parse(res->body, nullptr, false);
        if (body.is_discarded() || body["resultCount"].get<int>() == 0) { return {}; }
        return body["results"][0].value("artworkUrl100", std::string{});
    } catch (...) {}
    return {};
}

ITunesAlbum ITunes::getAlbum(int64_t albumId)
{
    ITunesAlbum album;
    try {
        auto res = m_httpClient.Get("/lookup?id=" + std::to_string(albumId) + "&entity=song");
        if (!res || res->status != 200) {
            std::cout << "ITunes: HTTP " << (res ? res->status : -1) << "\n";
            return album;
        }

        auto body = json::parse(res->body, nullptr, false);
        if (body.is_discarded()) {
            std::cout << "ITunes: failed to parse response\n";
            return album;
        }

        const int count = body["resultCount"].get<int>();
        for (int i = 0; i < count; ++i) {
            auto& item = body["results"][i];
            const std::string wrapperType = item.value("wrapperType", "");

            if (wrapperType == "collection") {
                album.albumId   = item.value("collectionId", int64_t{0});
                album.artistId  = item.value("artistId", int64_t{0});
                album.trackCount = item.value("trackCount", 0);
            } else if (wrapperType == "track") {
                album.tracks.push_back({
                    .trackId     = item.value("trackId", int64_t{0}),
                    .artistId    = item.value("artistId", int64_t{0}),
                    .trackNumber = item.value("trackNumber", 0),
                    .artist      = item.value("artistName", std::string{}),
                    .collection  = item.value("collectionName", std::string{}),
                    .track       = item.value("trackName", std::string{}),
                    .artworkUrl  = item.value("artworkUrl100", std::string{}),
                });
            }
        }
    } catch (const std::exception& e) {
        std::cout << "ITunes: " << e.what() << "\n";
    }
    return album;
}

std::vector<ITunesResult> ITunes::getTop25Songs(const std::string& region)
{
    std::vector<ITunesResult> results;
    try {
        const std::string path = "/api/v2/" + region + "/music/most-played/25/songs.json";

        auto res = m_httpClient2.Get(path);
        if (!res || res->status != 200) {
            std::cout << "ITunes: HTTP " << (res ? res->status : -1) << "\n";
            return results;
        }

        auto body = json::parse(res->body, nullptr, false);
        if (body.is_discarded()) {
            std::cout << "ITunes: failed to parse response\n";
            return results;
        }

        auto& items = body["feed"]["results"];
        results.reserve(items.size());
        auto toInt64 = [](const std::string& s) -> int64_t {
            try { return std::stoll(s); } catch (...) { return 0; }
        };
        for (auto& song : items) {
            results.push_back({
                .trackId    = toInt64(song.value("id", "0")),
                .artistId   = toInt64(song.value("artistId", "0")),
                .artist     = song.value("artistName", std::string{}),
                .track      = song.value("name", std::string{}),
                .artworkUrl = song.value("artworkUrl100", std::string{}),
            });
        }
    } catch (const std::invalid_argument& e) {
        std::cout << "ITunes: " << e.what() << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "ITunes: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "ITunes: " << e.what() << "\n";
    }
    return results;
}


std::string ITunes::formatTerm(const std::string& query)
{
    std::string result;
    result.reserve(query.size());
    for (char c : query) {
        result += (c == ' ') ? '+' : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}
