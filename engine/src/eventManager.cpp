#include "eventManager.h"

std::list<std::function<void()>> EventManager::m_events;

std::mutex EventManager::m_mutex;

void EventManager::ProcessEvent() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (!m_events.empty()) {
        auto &event = m_events.front();
        lock.unlock();

        event();

        lock.lock();
        m_events.pop_front();
    }
}

const u32 EventManager::getcount() {
    return 0;
}

