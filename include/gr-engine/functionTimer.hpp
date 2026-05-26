#pragma once

#include <chrono>

class FunctionTimer
{
public:
    FunctionTimer(const char* name);
    ~FunctionTimer();
    
private:
    std::chrono::high_resolution_clock::time_point m_start;

    const char* m_name;
};




