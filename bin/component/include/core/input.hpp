#pragma once

#include <gr-math/gmath.h>

enum InputKey : grm::u32 {
    InputKey_W = 1 << 1,
    InputKey_A = 1 << 2,
    InputKey_S = 1 << 3,
    InputKey_D = 1 << 4,
    InputKey_Space = 1 << 7,
    InputKey_Mouse_Left = 1 << 8,
    InputKey_Mouse_Right = 1 << 9,
    InputKey_Mouse_ButtonMiddle = 1 << 10,
};

class Input {
public:
    static bool IskeyPress(InputKey key);

    static void SetCursorPos(const Vector2 &value);

    static Vector2 GetCursorPos();

    static bool IsMousePress(InputKey key);
};




