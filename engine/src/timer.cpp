#include "timer.h"
#include "Global.h"

float Time::deltaTime = 0.0f;

Time::Time(float maxFPS) : a(std::chrono::system_clock::now()), b(std::chrono::system_clock::now()), m_targetFrameTime(1000.0 / static_cast<double>(maxFPS)) {}

float Time::SleepTick() {
    a = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> work_time = a - b;

    if (work_time.count() < m_targetFrameTime) {
        std::chrono::duration<double, std::milli> delta_ms(m_targetFrameTime - work_time.count());
        auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
    }
    b = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> sleep_time = b - a;
    return static_cast<float>((work_time.count() + sleep_time.count()) / 1e3f);
}

float Time::start() {
    a = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> sleep_time = a - b;
    return static_cast<float>(sleep_time.count() / 1e3f);
}

void Time::end() {
    b = std::chrono::system_clock::now();
}
