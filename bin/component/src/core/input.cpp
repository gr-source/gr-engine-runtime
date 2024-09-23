#include "core/input.hpp"

#include "api.hpp"

bool Input::IskeyPress(InputKey key) {
    return false;//InputIsKeyPress((grm::u32)key);
}

void Input::SetCursorPos(const Vector2 &value) {
    // InputSetCursourPos(value);
}

Vector2 Input::GetCursorPos() {
    return {};// InputGetCursorPos();
}

bool Input::IsMousePress(InputKey key) {
    return false; //InputIsMousePress((grm::u32) key);
}
