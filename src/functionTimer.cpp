#include "gr-engine/functionTimer.hpp"

FunctionTimer::FunctionTimer() : startTime(std::chrono::high_resolution_clock::now()), deltaTime(0.0f)
{
}

void FunctionTimer::start()
{
    startTime = std::chrono::high_resolution_clock::now();
}

void FunctionTimer::end()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration<float>(currentTime - startTime).count();
}

float FunctionTimer::getDeltaTime() const
{
    return deltaTime;
}

float FunctionTimer::getFps() const
{
    return (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
}


