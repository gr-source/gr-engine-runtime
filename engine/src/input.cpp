#include "input.h"

std::string getKeyName(KeyCode key) {
    switch (key) {
    case KeyCode::A:
        return "A";
    case KeyCode::W:
        return "W";
    case KeyCode::S:
        return "S";
    case KeyCode::D:
        return "D";
    case KeyCode::SPACE:
        return "SPACE";
    case KeyCode::NONE:
        return "NONE";
    default:
        return "";
    }
}

KeyCode getKeyCode(unsigned char c) {
    switch (c) {
    case 'A':
        return KeyCode::A;
    case 'W':
        return KeyCode::W;
    case 'S':
        return KeyCode::S;
    case 'D':
        return KeyCode::D;
    case ' ':
        return KeyCode::SPACE;
    default:
        return KeyCode::NONE;
    }
}

KeyAction getKeyAction(int c) {
    switch (c) {
    case 0:
        return KeyAction::UP;
    case 1:
        return KeyAction::DOWN;
    case 2:
        return KeyAction::PRESS;
    default:
        assert("fail key action do not exists");
    }
    return KeyAction::NONEN;
}

std::map<MouseButton, KeyAction> mouseMap = {
    {MouseButton::BUTTON_1, KeyAction::NONEN},
};

std::map<KeyCode, KeyAction> keyMap = {
    {KeyCode::A, KeyAction::NONEN},
    {KeyCode::W, KeyAction::NONEN},
    {KeyCode::S, KeyAction::NONEN},
    {KeyCode::D, KeyAction::NONEN},
    {KeyCode::SPACE, KeyAction::NONEN},
    {KeyCode::MIDDLE, KeyAction::NONEN},
};

int Input::IsKeyDown(KeyCode key) {
    return ((keyMap[key] == KeyAction::DOWN) ? true : false);
}

int Input::IsKeyUp(KeyCode key) {
    return ((keyMap[key] == KeyAction::UP) ? true : false);
}

int Input::IsKeyPress(KeyCode key) {
    return ((keyMap[key] == KeyAction::PRESS || keyMap[key] == KeyAction::DOWN || keyMap[key] == KeyAction::UP) ? true : false);
}

void Input::SetKeyAction(KeyCode key, KeyAction action) {
    keyMap[key] = action;
}

void Input::SetMouseAction(MouseButton button, KeyAction action) {
    mouseMap[button] = action;
}

int Input::IsMouseUp(MouseButton button) {
    return ((mouseMap[button] == KeyAction::UP) ? true : false);
}

int Input::IsMousePress(MouseButton button) {
    return ((mouseMap[button] == KeyAction::DOWN) ? true : false);
}

MouseButton Input::getMouseButton(int button) {
    switch (button) {
    case 0:
        return MouseButton::BUTTON_1;
    default:
        return MouseButton::NONE;
    }
}

Vector2 cursorPos;

void Input::SetCursorPos(float x, float y) {
    cursorPos = {x, y};
}

Vector2 Input::GetCursorPos() {
    return cursorPos;
}

bool lockedM = false;

void Input::SetLockMouse(bool locked) {
    lockedM = locked;
}

bool Input::IsLockMouse() {
    return lockedM;
}

