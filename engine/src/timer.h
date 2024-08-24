#pragma once

#include <chrono>

#include <time.h>

class TimeLast {
public:
    static float deltaTime;

    static float FPS;

    static float RenderFPS;

    TimeLast(float maxFPS = 60.0f);

    float SleepTick();

    void start();

    float end();

    const int getFPS() const;

protected:
    std::chrono::system_clock::time_point a;
    std::chrono::system_clock::time_point b;

    float m_targetFrameTime;
    float elapsedTime;
    int frameCount;
    int fps;

    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
};
