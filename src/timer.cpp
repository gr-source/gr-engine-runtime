#include "gr-engine/timer.hpp"

#include <GLFW/glfw3.h>
#include <thread>

Timer::Timer(double fps)
    : m_targetFrameTime(fps > 0.0 ? 1.0 / fps : 0.0),
      lastTime(glfwGetTime())
{}


void Timer::set_fps(double fps)
{
    m_targetFrameTime = fps > 0.0 ? 1.0 / fps : m_targetFrameTime;
}

void Timer::begin_frame()
{
    double nowTime = glfwGetTime();
    deltaTime = nowTime - lastTime;
    lastTime = nowTime;
}

void Timer::end_frame() const
{
    double endTime =
        glfwGetTime();

    double frameTime =
        endTime - lastTime;

    if (m_targetFrameTime > 0.0 && frameTime < m_targetFrameTime)
        std::this_thread::sleep_for(
                std::chrono::duration<double>(m_targetFrameTime - frameTime)
            );
}






