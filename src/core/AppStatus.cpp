#include "core/AppStatus.hpp"

AppStatus* AppStatus::m_instance = nullptr;

void AppStatus::set(const std::string& msg)
{
    std::lock_guard lock(m_mutex);
    m_message = msg;
    m_lastSet = std::chrono::steady_clock::now();
}

void AppStatus::clear()
{
    std::lock_guard lock(m_mutex);
    m_message.clear();
}

std::string AppStatus::get() const
{
    std::lock_guard lock(m_mutex);
    return m_message;
}

bool AppStatus::isActive() const
{
    std::lock_guard lock(m_mutex);
    if (m_message.empty()) { return false; }
    const auto elapsed = std::chrono::steady_clock::now() - m_lastSet;
    return std::chrono::duration<float>(elapsed).count() < kFadeSeconds;
}
