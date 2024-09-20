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

    static bool IsKeyDown(Key key);

    static bool IsKeyUp(Key key);

    static bool IsKeyPress(grm::u32 key);

    static bool IsMouseReleased(MouseButton button);

    static bool IsMouseDown(MouseButton button);

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
