#include "api/YouTube.hpp"

#include <format>
#include <iostream>
#include <json.hpp>

#include "core/Settings.hpp"

using json = nlohmann::json;

YouTube::YouTube()
    : m_httpClient("https://www.googleapis.com")
{}

std::optional<YouTubeResult> YouTube::search(const std::string& query)
{
    try {
        const std::string apiKey = Settings::Instance()->get<std::string>("youtube_api_key", "");
        if (apiKey == "") {
            std::cout << "Youtube: must have a API key set." << std::endl;
            return std::nullopt;
        }

        const std::string path = "/youtube/v3/search"
            "?part=snippet&type=video&videoCategoryId=10&maxResults=1&q="
            + httplib::encode_query_component(query)
            + "&key=" + apiKey;

        auto res = m_httpClient.Get(path);
        if (!res || res->status != 200) {
            std::cout << "YouTube: HTTP " << (res ? res->status : -1) << "\n";
            return std::nullopt;
        }

        auto body = json::parse(res->body, nullptr, false);
        if (body.is_discarded()) {
            std::cout << "YouTube: failed to parse response\n";
            return std::nullopt;
        }

        return YouTubeResult{
            .title   = body["items"][0]["snippet"]["title"].get<std::string>(),
            .videoId = body["items"][0]["id"]["videoId"].get<std::string>(),
        };
    } catch (const std::exception& e) {
        std::cout << "YouTube: " << e.what() << "\n";
    }
    return std::nullopt;
}

bool YouTube::download(const YouTubeResult& target, const std::string& outputPath)
{
    const std::string cmd = std::format(
        "./bin/yt-dlp -x --audio-format mp3 -o \"{}\" \"https://youtube.com/watch?v={}\"",
        outputPath, target.videoId
    );

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) { return false; }

    char buf[256];
    while (fgets(buf, sizeof(buf), pipe)) {}

    return pclose(pipe) == 0;
}
