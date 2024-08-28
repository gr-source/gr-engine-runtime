#include "settings.h"

bool Settings::m_bEngineRunning = false;

bool Settings::IsGameRunning = false;

Vector2 Settings::GameGrapicQiality = {1920.0f, 1080.0f};

Vector2 Settings::IconSize = {32.0f, 32.0f};

iVector2 Settings::ViewportSize = {0, 0};

std::mutex Settings::m_mutex;

bool Settings::IsEngineRunning() {
    // std::lock_guard<std::mutex> lock(m_mutex);
    return m_bEngineRunning;
}

void Settings::StartPlayEngine() {
    // std::lock_guard<std::mutex> lock(m_mutex);
    m_bEngineRunning = true;
}

void Settings::StopPlayEngine() {
    // std::lock_guard<std::mutex> lock(m_mutex);
    m_bEngineRunning = false;
}

