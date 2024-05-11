#pragma once

#include <chrono>

#include <time.h>

class Time {
public:
    static float deltaTime;

    Time(float fps = 60.0f);

    float start() const;

    void end();

    void tick();

    float dt;

protected:
    std::chrono::_V2::steady_clock::time_point lastTime;
    std::chrono::_V2::steady_clock::time_point endTime;
    const std::chrono::duration<float> targetFPS;
};
