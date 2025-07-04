#include "api.hpp"

#include <gr-base/core/event.hpp>

#include "core/time.hpp"

void SetDeltaTime(float deltaTime) {
    Time::deltaTime = deltaTime;
}



