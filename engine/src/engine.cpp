#include "engine.h"

Engine::GRHandleEvent Engine::m_handleEvent = nullptr;

std::string Engine::m_error;

Monitor Engine::monitor;

bool Engine::Initialize(GRHandleEvent callback) {
    if (!callback) {
        return false;
    }
    m_handleEvent = callback;

    Engine::SetHandleEvent(Engine::GRINITIALIZE);
    
    Plataform::Initialize(monitor);
    return true;
}

void Engine::SetHandleEvent(Event event) {
    m_handleEvent(event);
}

void Engine::SetError(const std::string &error) {
    m_error = error;
}

const std::string Engine::GetError() {
    return m_error;
}
