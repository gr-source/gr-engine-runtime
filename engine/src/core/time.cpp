#include "time.hpp"

#include <GLFW/glfw3.h>
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

    return dt;
}

void Time::end() {
    m_end = m_start;
}

float Time::sleep(float fps) {
    float target_frame_time = 1000.0f / fps;

    m_end = std::chrono::system_clock::now();
    std::chrono::duration<float, std::milli> work_time = m_end - m_start;

    if (work_time.count() < target_frame_time) {
        std::chrono::duration<float, std::milli> delta_ms(target_frame_time - work_time.count());

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


gtimer::gtimer(float fps) : max_fps(fps), min_frame_time(1.0 / fps), last_time(glfwGetTime()), delta(1.0f)
{}

void gtimer::wait()
{
    double current_time = glfwGetTime();
    delta = static_cast<float>(current_time - last_time);

    if (delta <= min_frame_time)
    {
        double sleep_time = min_frame_time - delta;
        std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));

        current_time = glfwGetTime();
        delta = static_cast<float>(current_time - last_time);
    }

    last_time = current_time;
}

float gtimer::get_delta() const
{
    return delta;
}




