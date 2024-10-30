#include "core/event.hpp"

#include "api.hpp"

void Event::AddEvent_Internal(std::function<void()> func) {
    EventAddEvent_Internal(func);
}

