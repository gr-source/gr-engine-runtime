#include "time.hpp"

#include <thread>

Time::Time() : m_start(std::chrono::system_clock::now()), m_end(std::chrono::system_clock::now()), m_countTime(0.0f), m_frameCount(0), m_fps(0.0f) {}

float Time::start() {
    m_start = std::chrono::system_clock::now();

    std::chrono::duration<float> duration = m_start - m_end;
    auto dt = duration.count();

    ++m_frameCount;

    m_countTime += dt;
    if (m_countTime >= 1.0f) {
        m_fps = static_cast<float>(m_frameCount) / m_countTime;
        m_frameCount = 0;
        m_countTime = 0;
    }

    m_end = m_start;

    return dt;
}

void Time::end() {
    // m_end = m_start;//std::chrono::system_clock::now();
}

#include <iostream>

float Time::sleep(float fps) {
    auto m_targetFrameTime(1000.0f / fps);

    m_end = std::chrono::system_clock::now();
    std::chrono::duration<float, std::milli> work_time = m_end - m_start;

    if (work_time.count() < m_targetFrameTime) {
        std::chrono::duration<float, std::milli> delta_ms(m_targetFrameTime - work_time.count());
        auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
    }
    
    m_start = std::chrono::system_clock::now();
    std::chrono::duration<float, std::milli> sleep_time = m_start - m_end;
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(work_time + sleep_time);

    ++m_frameCount;
    m_countTime += elapsed.count() / 1e9f;
    if (m_countTime >= 1.0f) {
        m_fps = static_cast<float>(m_frameCount) / m_countTime;
        m_frameCount = 0;
        m_countTime = 0.0f;
    }
    return elapsed.count() / 1e9f;
}

float Time::getFPS() {
    return m_fps;
}
