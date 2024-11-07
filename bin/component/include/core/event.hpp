#pragma once

#include <functional>

class Event {
public:
    template <typename ...Args>
    static void AddEvent(Args &&...args);

    static void (*AddInternalEvent)(std::function<void()> func);
};

template <typename... Args>
inline void Event::AddEvent(Args &&...args) {
    return AddInternalEvent(std::bind(std::forward<Args>(args)...));
}
