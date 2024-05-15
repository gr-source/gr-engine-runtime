#pragma once

#include <gr-math/vector2.h>

struct Settings {
    static bool IsEngineRunning;

    static bool IsGameRunning;

    static Vector2 GameGrapicQiality;

    static Vector2 IconSize;

    static Vector2 ViewportSize;
};

template <typename T, typename R>
inline T reinterpret_vector(const R& lhs) {
    return {lhs.x, lhs.y};
}

