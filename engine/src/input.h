#pragma once

#include <unordered_map>
#include <string>

#include <gr-math/gmath.h>

enum MouseButton {
    MouseButton_Right = 1 << 1,
    MouseButton_Left = 1 << 2,
};

class Input {
public:
    enum Key {
        InputKey_W = 1 << 1,
        InputKey_A = 1 << 2,
        InputKey_S = 1 << 3,
        InputKey_D = 1 << 4,
        InputKey_Space = 1 << 7,
        InputKey_Mouse_Left = 1 << 8,
        InputKey_Mouse_Right = 1 << 9,
        InputKey_Mouse_ButtonMiddle = 1 << 10,
    };

    static std::string getKeyName(Key key);

    static bool IsKeyDown(Key key);

    static bool IsKeyUp(Key key);

    static bool IsKeyPress(grm::u32 key);

    static bool IsMouseReleased(Key button);

    static bool IsMouseDown(Key button);

    static void SetCursorPos(const Vector2& position);

    static Vector2 GetCursorPos();

    static void Initialize();

    static void Update();
    
private:
    struct MouseData {
        Vector2 pos;
        bool want;
    };

    static std::unordered_map<MouseButton, bool> m_mouseButtonState;

    static MouseData m_mouseData;
};
