#include "gr-engine/timer.hpp"

#include <GLFW/glfw3.h>
#include <thread>

Timer::Timer(double fps)
    : m_targetFrameTime(fps > 0.0 ? 1.0 / fps : 0.0),
      m_lastTime(glfwGetTime()),
      m_delta(0.0)
{}


void Timer::set_fps(double fps)
{
    m_targetFrameTime = fps > 0.0 ? 1.0 / fps : m_targetFrameTime;
}

void Timer::begin_frame()
{
    double now = glfwGetTime();
    m_delta = now - m_lastTime;
    m_lastTime = now;
}

void Timer::end_frame()
{
    double frameTime = glfwGetTime() - m_lastTime;

    if (m_targetFrameTime > 0.0 && frameTime < m_targetFrameTime)
    {
        double sleep = m_targetFrameTime - frameTime;
        std::this_thread::sleep_for(std::chrono::duration<double>(sleep));
    }
}

double Timer::get_delta() const
{
    return m_delta;
}





