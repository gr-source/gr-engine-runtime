#pragma once

#include <chrono>

class FunctionTimer
{
public:
    FunctionTimer();

    void start();
    
    void end();
    
    float getDeltaTime() const;
    
    float getFps() const;

private:
    std::chrono::high_resolution_clock::time_point startTime;

    float deltaTime;
};




