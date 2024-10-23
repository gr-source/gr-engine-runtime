#include "core/input.hpp"

#include "api.hpp"

bool Input::IskeyPress(InputKey key) {
    return InputIsKeyPress(key);
}

void Input::SetCursorPos(const Vector2 &value) {
    throw std::runtime_error("function do not implemented.");
}

Vector2 Input::GetCursorPos() {
    throw std::runtime_error("function do not implemented.");
}

bool Input::IsMousePress(InputKey key) {
    throw std::runtime_error("function do not implemented.");
}




