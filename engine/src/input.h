#pragma once

#include <cassert>
#include <string>
#include <map>

<<<<<<< HEAD
#include <linux/input-event-codes.h>

=======
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
#include <gr-math/gmath.h>

#include "types.h"

class Input {
public:
<<<<<<< HEAD
=======
    enum Key : u32 {
        NONE = 1 << 0,
        KEY_W = 1 << 1,
        KEY_A = 1 << 2,
        KEY_S = 1 << 3,
        KEY_D = 1 << 4,
        KEY_UP = 1 << 5,
        KEY_DOWN = 1 << 6,
        KEY_SPACE = 1 << 7,
        BUTTON_MIDDLE = 1 << 8
    };

    static std::string getKeyName(Key key);

    Key getKeyAction(int c);

    static int IsKeyDown(Key key);

    static int IsKeyUp(Key key);

    static int IsKeyPress(Key key);

    static int IsMouseUp(Key button);

    static int IsMousePress(Key button);

    static Key getMouseButton(int button);

    static void SetCursorPos(const Vector2& position);

    static Vector2 GetCursorPos();

    static void SetInstance(void* instance);
private:
    static void* m_instance;
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
};
