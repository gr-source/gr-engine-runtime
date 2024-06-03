#include "engine.h"

Engine::GRHandleEvent Engine::m_handleEvent = nullptr;

std::string Engine::m_error;

bool Engine::Initialize(GRHandleEvent callback) {
    if (!callback) {
        return false;
    }
    m_handleEvent = callback;

    GParams params;

    Engine::SetHandleEvent(Engine::GR_CREATEWINDOW, &params);
    
    Plataform::Initialize(params.window);
    return true;
}

void* Engine::SetHandleEvent(Event event, GParams* params) {
    return m_handleEvent(event, params);
}

void Engine::SetError(const std::string &error) {
    m_error = error;
}

const std::string Engine::GetError() {
    return m_error;
}
