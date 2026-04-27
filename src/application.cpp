#include "gr-engine/application.hpp"

#include "gr-engine/engine.hpp"

Application* Application::GetInstance()
{
    static Engine s_Engine;
    return &s_Engine;
}
