#pragma once

#include <chrono>

#include <time.h>

class Time {
public:
    static float deltaTime;

    Time(float maxFPS = 60.0f);

    float SleepTick();

    float start();

    void end();

protected:
    std::chrono::system_clock::time_point a;
    std::chrono::system_clock::time_point b;
    double m_targetFrameTime;
};
