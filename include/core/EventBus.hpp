#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <any>

class EventBus
{
public:
    template<typename... Args, typename F>
    void on(const std::string& event, F&& handler)
    {
        m_handlers[event].push_back(std::function<void(Args...)>(std::forward<F>(handler)));
    }

    // void events
    void emit(const std::string& event) const
    {
        auto it = m_handlers.find(event);
        if (it == m_handlers.end()) { return; }
        for (auto& h : it->second)
        {
            auto* fn = std::any_cast<std::function<void()>>(&h);
            if (fn) { (*fn)(); }
        }
    }

    // single-arg events
    template<typename T>
    void emit(const std::string& event, const T& arg) const
    {
        auto it = m_handlers.find(event);
        if (it == m_handlers.end()) { return; }
        for (auto& h : it->second)
        {
            auto* fn = std::any_cast<std::function<void(T)>>(&h);
            if (fn) { (*fn)(arg); }
        }
    }

    static EventBus* Instance() { return m_instance; }
    static EventBus* m_instance;

private:
    std::unordered_map<std::string, std::vector<std::any>> m_handlers;
};
