#pragma once

#include <functional>
#include <algorithm>
#include <string>
#include <mutex>
#include <list>

#include "types.h"

class Event {
public:
    
    template <typename ...Args>
    static void Add(Args &&...args);

    static void ProcessEvent();

    static const u32 getcount();
protected:
    static std::list<std::function<void()>> m_events;
    static std::mutex m_mutex;
};

template <typename... Args>
inline void Event::Add(Args&&... args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.emplace_back(std::bind(std::forward<Args>(args)...));
}

