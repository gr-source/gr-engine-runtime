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


class gtimer
{
public:
    gtimer(float fps = 60.0f);

    void wait();

    float get_delta() const;

private:
    float max_fps;
    
    double min_frame_time;
    
    double last_time;

    float delta;
};

