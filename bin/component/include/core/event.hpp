#pragma once

#include <functional>
#include <mutex>
#include <list>

class Event {
public:
    template <typename ...Args>
    static void AddEvent(Args &&...args);

    static void Update();

    static void Shutdown();
private:
    static std::list<std::function<void()>> s_events;

    static std::mutex s_mutex;
};

template <typename... Args>
inline void Event::AddEvent(Args &&...args) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_events.emplace_back(std::bind(std::forward<Args>(args)...));
}
