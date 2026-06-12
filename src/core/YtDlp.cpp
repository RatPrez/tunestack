#include "core/YtDlp.hpp"

#include <thread>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#include "core/AppStatus.hpp"

YtDlp* YtDlp::m_instance = nullptr;

#ifdef _WIN32
    static constexpr const char* kFilename = "yt-dlp.exe";
    static constexpr const char* kDownloadPath = "/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe";
#else
    static constexpr const char* kFilename = "yt-dlp";
    static constexpr const char* kDownloadPath = "/yt-dlp/yt-dlp/releases/latest/download/yt-dlp";
#endif

YtDlp::YtDlp(const std::filesystem::path& configDir)
    : m_path(configDir / kFilename)
{
    m_instance = this;
}

std::string YtDlp::getPath() const
{
    return m_ready ? m_path.string() : std::string{};
}

void YtDlp::ensureAvailable()
{
    if (std::filesystem::exists(m_path)) {
        m_ready = true;
        return;
    }

    if (m_downloading.exchange(true)) { return; }
    std::thread([this]() { download(); }).detach();
}

void YtDlp::download()
{
    AppStatus::Instance()->set("Downloading yt-dlp...");

    httplib::Client client("https://github.com");
    client.set_follow_location(true);
    client.set_connection_timeout(30);
    client.set_read_timeout(120);

    const auto res = client.Get(kDownloadPath);
    if (!res) {
        const auto err = res.error();
        AppStatus::Instance()->set(std::string("yt-dlp download failed: ") + httplib::to_string(err));
        std::cerr << "YtDlp: " << httplib::to_string(err) << "\n";
        m_downloading = false;
        return;
    }
    if (res->status != 200) {
        AppStatus::Instance()->set("yt-dlp download failed: HTTP " + std::to_string(res->status));
        std::cerr << "YtDlp: HTTP " << res->status << "\n" << res->body.substr(0, 200) << "\n";
        m_downloading = false;
        return;
    }

    // write to a temp file then rename so we never leave a partial binary
    const auto tmp = std::filesystem::path(m_path.string() + ".tmp");
    {
        std::ofstream f(tmp, std::ios::binary);
        if (!f) {
            AppStatus::Instance()->set("yt-dlp: could not write to config dir");
            m_downloading = false;
            return;
        }
        f.write(res->body.data(), (std::streamsize)res->body.size());
    }

#ifndef _WIN32
    // make executable
    std::filesystem::permissions(tmp,
        std::filesystem::perms::owner_exec |
        std::filesystem::perms::group_exec |
        std::filesystem::perms::others_exec,
        std::filesystem::perm_options::add);
#endif

    std::filesystem::rename(tmp, m_path);
    m_ready = true;
    m_downloading = false;
    AppStatus::Instance()->clear();
}
