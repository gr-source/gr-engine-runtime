#pragma once

#include <gr-math/gmath.hpp>

enum InputKey : int {
    InputKey_A,
    InputKey_B,
    InputKey_C,
    InputKey_D,
    InputKey_E,
    InputKey_F,
    InputKey_G,
    InputKey_H,
    InputKey_I,
    InputKey_J,
    InputKey_K,
    InputKey_L,
    InputKey_M,
    InputKey_N,
    InputKey_O,
    InputKey_P,
    InputKey_Q,
    InputKey_R,
    InputKey_S,
    InputKey_T,
    InputKey_U,
    InputKey_V,
    InputKey_W,
    InputKey_X,
    InputKey_Y,
    InputKey_Z,
    InputKey_Space,
    InputKey_Enter,
    InputKey_Escape,
    InputKey_Arrow_Up,
    InputKey_Arrow_Down,
    InputKey_Arrow_Left,
    InputKey_Arrow_Right,
    InputKey_Mouse_Left,
    InputKey_Mouse_Right,
    InputKey_Mouse_ButtonMiddle,
    InputKey_LeftCtrl,
    InputKey_RightCtrl,
    InputKey_LeftShift,
    InputKey_RightShift,
    InputKey_LeftAlt,
    InputKey_RightAlt
};

class Input {
public:
    static bool (*IskeyPress)(InputKey key);

    static void (*SetCursorPos)(const Vector2 &value);

    static Vector2 (*GetCursorPos)();

    static bool (*IsMousePress)(InputKey key);
};




