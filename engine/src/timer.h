#pragma once

#include <chrono>

#include <time.h>

class Time {
public:
    static float deltaTime;

    static int FPS;

    Time(float maxFPS = 60.0f);

    float SleepTick();

    void start();

    const float end();

    const int getFPS() const;

protected:
    std::chrono::system_clock::time_point a;
    std::chrono::system_clock::time_point b;
    double m_targetFrameTime;
    float elapsedTime;
    int frameCount;
    int fps;

    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
};
