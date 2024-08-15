#include "input.h"

#include <GLFW/glfw3.h>

#define GET_INSTANCE_PRESS(key) glfwGetKey((GLFWwindow*)Input::m_instance, key);
#define GET_INSTANCE_MOUSE_BUTTON(key) glfwGetMouseButton((GLFWwindow*)Input::m_instance, key);
#define GET_INSTANCE_MOUSE_POS() double x, y; glfwGetCursorPos((GLFWwindow*)Input::m_instance, &x, &y); return {static_cast<float>(x), static_cast<float>(y)};
#define SET_INSTANCE_MOUSE_POS(pos) glfwSetCursorPos((GLFWwindow*)Input::m_instance, static_cast<float>(pos.x), static_cast<float>(pos.y))

void* Input::m_instance = nullptr;

int get_key_to_glfw(grm::u32 key) {
    switch (key) {
    case Input::KEY_W:
        return GLFW_KEY_W;
    case Input::KEY_A:
        return GLFW_KEY_A;
    case Input::KEY_S:
        return GLFW_KEY_S;
    case Input::KEY_D:
        return GLFW_KEY_D;
    case Input::KEY_UP:
        return GLFW_KEY_UP;
    case Input::KEY_DOWN:
        return GLFW_KEY_DOWN;
    case Input::BUTTON_LAST:
        return GLFW_MOUSE_BUTTON_LAST;
    case Input::BUTTON_LEFT:
        return GLFW_MOUSE_BUTTON_LEFT;
    case Input::BUTTON_RIGHT:
        return GLFW_MOUSE_BUTTON_RIGHT;
    case Input::BUTTON_MIDDLE:
        return GLFW_MOUSE_BUTTON_MIDDLE;
    default:
        return GLFW_INVALID_ENUM;
    }
}

std::string Input::getKeyName(Key key) {
    #define GET_ENUM_NAME(e) case e: return #e;
    switch (key) {
    GET_ENUM_NAME(KEY_A)
    GET_ENUM_NAME(KEY_W)
    GET_ENUM_NAME(KEY_S)
    GET_ENUM_NAME(KEY_D)
    GET_ENUM_NAME(KEY_SPACE)
    default:
        GET_ENUM_NAME(NONE)
    }
    #undef GET_ENUM_NAME
}

const bool Input::IsKeyDown(Key key) {
    return 0;
}

const bool Input::IsKeyUp(Key key) {
    return 0;
}

const bool Input::IsKeyPress(grm::u32 key) {
    return GET_INSTANCE_PRESS(get_key_to_glfw(key));
}

const bool Input::IsMouseUp(Key button) {
    return glfwGetMouseButton((GLFWwindow*)m_instance, get_key_to_glfw(button)) == GLFW_PRESS;
}

const bool Input::IsMousePress(grm::u32 button) {
    return 0;
}

void Input::SetCursorPos(const Vector2& position) {
    SET_INSTANCE_MOUSE_POS(position);
}

const Vector2 Input::GetCursorPos() {
    GET_INSTANCE_MOUSE_POS();
}

void Input::SetInstance(void *instance) {
    m_instance = instance;
}


