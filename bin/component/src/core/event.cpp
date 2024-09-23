#include "core/event.hpp"

std::list<std::function<void()>> Event::s_events;

std::mutex Event::s_mutex;

void Event::Update() {
    std::unique_lock<std::mutex> lock(s_mutex);
    while (!s_events.empty()) {
        auto& func = s_events.front();
        lock.unlock();  

        func();

        lock.lock();
        s_events.pop_front();
    }
}

void Event::Shutdown() {
    s_events.clear();
}
