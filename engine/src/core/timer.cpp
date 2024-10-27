#include "timer.hpp"

#include "Global.h"

#include <thread>

float TimeLast::deltaTime = 0.0f;
float TimeLast::FPS = 0;

float TimeLast::RenderFPS = 0;

TimeLast::TimeLast(float maxFPS) : a(std::chrono::system_clock::now()), b(std::chrono::system_clock::now()), m_targetFrameTime(1000.0f / maxFPS), elapsedTime(0.0f), frameCount(0), fps(0) {}

float TimeLast::SleepTick() {
    a = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> work_time = a - b;

    if (work_time.count() < m_targetFrameTime) {
        std::chrono::duration<double, std::milli> delta_ms(m_targetFrameTime - work_time.count());
        auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
    }
    
    b = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> sleep_time = b - a;
    float dt = static_cast<float>((work_time.count() + sleep_time.count()) / 1e3f);

    ++frameCount;
    elapsedTime += dt;
    if (elapsedTime >= 1.0000) {
        fps = frameCount / elapsedTime;
        frameCount = 0;
        elapsedTime = 0.0f;
    }
    return dt;
}

void TimeLast::start() {
    start_time = std::chrono::system_clock::now();
}

float TimeLast::end() {
    end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration<float>(end_time - start_time);
    return duration.count();
}

const int TimeLast::getFPS() const {
    return fps;
}
