#pragma once

#include <chrono>

class Time {
public:
    Time();

    float start();

    void end();

    float sleep(float fps);

    float getFPS();

private:
    std::chrono::system_clock::time_point m_start;
    std::chrono::system_clock::time_point m_end;

    float m_countTime;

    int m_frameCount;

    float m_fps;
};
