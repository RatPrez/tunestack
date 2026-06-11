#pragma once

#include <chrono>
#include <mutex>
#include <string>

class AppStatus
{
public:
    AppStatus() { m_instance = this; }

    void set(const std::string& msg);
    void clear();
    std::string get() const;
    bool isActive() const;  // false after kFadeSeconds of inactivity

    static AppStatus* Instance() { return m_instance; }

private:
    static constexpr float kFadeSeconds = 4.0f;
    static AppStatus* m_instance;

    mutable std::mutex m_mutex;
    std::string m_message;
    std::chrono::steady_clock::time_point m_lastSet;
};
