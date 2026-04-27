#pragma once

#include <string>

class Application
{
public:
    static Application* GetInstance();

    Application() = default;
    virtual ~Application() {}

    virtual void initialize() = 0;

    virtual void shutdown() = 0;

    virtual void process() = 0;

    virtual void DropFile(const std::string &path) {}
};


