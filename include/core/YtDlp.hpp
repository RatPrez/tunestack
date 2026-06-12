#pragma once

#include <atomic>
#include <filesystem>
#include <string>

class YtDlp
{
public:
    explicit YtDlp(const std::filesystem::path& configDir);

    // returns the usable path, or empty string if not yet available
    std::string getPath() const;
    bool isReady() const { return m_ready; }

    // kicks off background download if not already present; safe to call every launch
    void ensureAvailable();

    static YtDlp* Instance() { return m_instance; }

private:
    void download();

    std::filesystem::path m_path;
    std::atomic<bool> m_ready { false };
    std::atomic<bool> m_downloading { false };

    static YtDlp* m_instance;
};
