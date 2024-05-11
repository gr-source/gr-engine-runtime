#pragma once

#include <cassert>
#include <string>
#include <map>

#include <gr-math/gmath.h>

enum KeyCode {
    W = 0x01,
    A = 0x02,
    S = 0x03,
    D = 0x04,
    SPACE = 0x005,
    MIDDLE = 0x006,
    NONE = 0x00
};

enum KeyAction {
    DOWN = 0x0001,
    UP = 0x0002,
    PRESS = 0x0003,
    NONEN = 0xffff
};

enum struct MouseButton {
    BUTTON_1,
    NONE
};

std::string getKeyName(KeyCode key);

KeyCode getKeyCode(unsigned char c);

KeyAction getKeyAction(int c);

namespace Input {
    int IsKeyDown(KeyCode key);

    int IsKeyUp(KeyCode key);

    int IsKeyPress(KeyCode key);

    void SetKeyAction(KeyCode key, KeyAction action);

    void SetMouseAction(MouseButton button, KeyAction action);

    int IsMouseUp(MouseButton button);

    int IsMousePress(MouseButton button);

    MouseButton getMouseButton(int button);

    void SetCursorPos(float x, float y);

    Vector2 GetCursorPos();

    void SetLockMouse(bool locked);

    bool IsLockMouse();
};
