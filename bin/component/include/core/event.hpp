#pragma once

#include <functional>
#include <mutex>
#include <list>

class Event {
public:
    template <typename ...Args>
    static void AddEvent(Args &&...args);

private:
    static void AddEvent_Internal(std::function<void()> func);
};

template <typename... Args>
inline void Event::AddEvent(Args &&...args) {
    AddEvent_Internal(std::bind(std::forward<Args>(args)...));
}
