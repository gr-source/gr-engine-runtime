#pragma once

#include <gr-math/gmath.h>

enum InputKey {
    InputKey_W = 1 << 1,
    InputKey_A = 1 << 2,
    InputKey_S = 1 << 3,
    InputKey_D = 1 << 4,
    InputKey_Space = 1 << 7,
    InputKey_ButtonMiddle = 1 << 8,
};

class Input {
public:
    static bool IskeyPress(InputKey key);

    static void SetCursorPos(const Vector2 &value);

    static Vector2 GetCursorPos();

    static bool IsMousePress(InputKey key);
};




