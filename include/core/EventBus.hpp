#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class EventBus
{
public:
    using Handler = std::function<void()>;

    void on(const std::string& event, Handler handler)
    {
        m_handlers[event].push_back(std::move(handler));
    }

    void emit(const std::string& event) const
    {
        auto it = m_handlers.find(event);
        if (it == m_handlers.end()) { return; }
        for (auto& handler : it->second) { handler(); }
    }

    static EventBus* Instance() { return m_instance; }
    static EventBus* m_instance;

private:
    std::unordered_map<std::string, std::vector<Handler>> m_handlers;
};
