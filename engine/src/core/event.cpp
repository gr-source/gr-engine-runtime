#include "event.hpp"

std::list<std::function<void()>> Event::m_events;

std::mutex Event::m_mutex;

void Event::ProcessEvent() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (!m_events.empty()) {
        auto &event = m_events.front();
        lock.unlock();

        event();

        lock.lock();
        m_events.pop_front();
    }
}

const u32 Event::getcount() {
    return 0;
}

