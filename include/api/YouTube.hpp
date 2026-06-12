#pragma once

#include <optional>
#include <string>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

struct YouTubeResult
{
    std::string title;
    std::string videoId;
};

enum class DownloadResult { Ok, RateLimited, NotFound, Error };

struct SearchResult
{
    std::optional<YouTubeResult> result;
    bool rateLimited = false;
};

class YouTube
{
public:
    YouTube();
    SearchResult search(const std::string& query);
    DownloadResult download(const YouTubeResult& target, const std::string& outputPath);

private:
    httplib::Client m_httpClient;
};
