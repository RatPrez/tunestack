#include "core/Settings.hpp"

#include <fstream>

Settings* Settings::m_instance = nullptr;

Settings::Settings()
{
    m_instance = this;
    load();
}

std::filesystem::path Settings::configPath() const
{
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    return std::filesystem::path(appdata ? appdata : ".") / "tunestack" / "settings.json";
#elif __APPLE__
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home ? home : ".") / "Library" / "Application Support" / "tunestack" / "settings.json";
#else
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    if (xdg) return std::filesystem::path(xdg) / "tunestack" / "settings.json";
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home ? home : ".") / ".config" / "tunestack" / "settings.json";
#endif
}

void Settings::load()
{
    const auto path = configPath();
    if (!std::filesystem::exists(path)) { return; }

    std::ifstream file(path);
    if (!file.is_open()) { return; }

    try { m_data = nlohmann::json::parse(file); }
    catch (...) { m_data = {}; }
}

void Settings::save() const
{
    const auto path = configPath();
    std::filesystem::create_directories(path.parent_path());

    std::ofstream file(path);
    if (!file.is_open()) { return; }

    file << m_data.dump(2);
}
