#pragma once

#include <cassert>
#include <string>
#include <map>

#include <gr-math/gmath.h>

class Input {
public:
    enum Key : grm::u32 {
        NONE = 1 << 0,
        KEY_W = 1 << 1,
        KEY_A = 1 << 2,
        KEY_S = 1 << 3,
        KEY_D = 1 << 4,
        KEY_UP = 1 << 5,
        KEY_DOWN = 1 << 6,
        KEY_SPACE = 1 << 7,
        BUTTON_MIDDLE = 1 << 8,
        BUTTON_RIGHT = 1 << 9,
        BUTTON_LEFT = 1 << 10,
        BUTTON_LAST = 1 << 11
    };

    static std::string getKeyName(Key key);

    static const bool IsKeyDown(Key key);

    static const bool IsKeyUp(Key key);

    static const bool IsKeyPress(grm::u32 key);

    static const bool IsMouseUp(Key button);

    static const bool IsMousePress(grm::u32 button);

    static void SetCursorPos(const Vector2& position);

    static const Vector2 GetCursorPos();

    static void SetInstance(void* instance);

    static void* m_instance;
private:
};
