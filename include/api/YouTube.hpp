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

class YouTube
{
public:
    YouTube();
    std::optional<YouTubeResult> search(const std::string& query);
    DownloadResult download(const YouTubeResult& target, const std::string& outputPath);

private:
    httplib::Client m_httpClient;
};
