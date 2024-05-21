#pragma once

#include <algorithm>
#include <string>
#include <mutex>
#include <list>

#include "types.h"

class EventManager {
public:
    
    template <typename ...Args>
    static void addEvent(Args&&... args);

    static void ProcessEvent();

    static const u32 getcount();
protected:
    static std::list<std::function<void()>> m_events;
    static std::mutex m_mutex;
};

template <typename... Args>
inline void EventManager::addEvent(Args&&... args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.emplace_back(std::bind(std::forward<Args>(args)...));
}

