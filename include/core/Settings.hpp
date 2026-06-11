#pragma once

#include <filesystem>
#include <string>

#include <json.hpp>

class Settings
{
public:
    Settings();

    void load();
    void save() const;

    template<typename T>
    T get(const std::string& key, const T& fallback = T{}) const
    {
        if (!m_data.contains(key)) { return fallback; }
        return m_data[key].get<T>();
    }

    template<typename T>
    void set(const std::string& key, const T& value)
    {
        m_data[key] = value;
        save();
    }

    static Settings* Instance() { return m_instance; }

private:
    std::filesystem::path configPath() const;

    nlohmann::json m_data;
    static Settings* m_instance;
};
