#pragma once

#include <mutex>

#include <gr-math/gmath.h>

class Settings {
public:
    static bool IsEngineRunning();

    static void StartPlayEngine();

    static void StopPlayEngine();

    static bool IsGameRunning;

    static Vector2 GameGrapicQiality;

    static Vector2 IconSize;

    static iVector2 ViewportSize;

private:
    static std::mutex m_mutex;

    static bool m_bEngineRunning;
};

