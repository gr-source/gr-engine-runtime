#include "input.h"
#include <GLFW/glfw3.h>

// Declaration of static member variable
void* Input::m_instance = nullptr;

// Helper function to convert Input::Key to GLFW key code
int get_key_to_glfw(Input::Key key) {
    switch (key) {
    case Input::KEY_W:        return GLFW_KEY_W;
    case Input::KEY_A:        return GLFW_KEY_A;
    case Input::KEY_S:        return GLFW_KEY_S;
    case Input::KEY_D:        return GLFW_KEY_D;
    case Input::KEY_SPACE:    return GLFW_KEY_SPACE;
    case Input::KEY_UP:       return GLFW_KEY_UP;
    case Input::KEY_DOWN:     return GLFW_KEY_DOWN;
    case Input::BUTTON_LAST:  return GLFW_MOUSE_BUTTON_LAST;
    case Input::BUTTON_LEFT:  return GLFW_MOUSE_BUTTON_LEFT;
    case Input::BUTTON_RIGHT: return GLFW_MOUSE_BUTTON_RIGHT;
    case Input::BUTTON_MIDDLE:return GLFW_MOUSE_BUTTON_MIDDLE;
    default:                  return GLFW_KEY_UNKNOWN; // Changed to a valid GLFW key
    }
}

// Helper function to get string name of Input::Key
std::string Input::getKeyName(Key key) {
    switch (key) {
    case KEY_A:     return "KEY_A";
    case KEY_W:     return "KEY_W";
    case KEY_S:     return "KEY_S";
    case KEY_D:     return "KEY_D";
    case KEY_SPACE: return "KEY_SPACE";
    default:        return "NONE";
    }
}

// Checks if a key is currently down
const bool Input::IsKeyDown(Key key) {
    return (glfwGetKey(static_cast<GLFWwindow*>(m_instance), get_key_to_glfw(key)) == GLFW_PRESS);
}

// Checks if a key is currently up
const bool Input::IsKeyUp(Key key) {
    return (glfwGetKey(static_cast<GLFWwindow*>(m_instance), get_key_to_glfw(key)) == GLFW_RELEASE);
}

// Checks if a key was pressed this frame
const bool Input::IsKeyPress(Key key) {
    return (glfwGetKey(static_cast<GLFWwindow*>(m_instance), get_key_to_glfw(key)) == GLFW_PRESS);
}

// Checks if a mouse button is currently up
const bool Input::IsMouseUp(Key button) {
    return (glfwGetMouseButton(static_cast<GLFWwindow*>(m_instance), get_key_to_glfw(button)) == GLFW_RELEASE);
}

// Checks if a mouse button was pressed this frame
const bool Input::IsMousePress(Key button) {
    return (glfwGetMouseButton(static_cast<GLFWwindow*>(m_instance), get_key_to_glfw(button)) == GLFW_PRESS);
}

// Sets the cursor position
void Input::SetCursorPos(const Vector2& position) {
    glfwSetCursorPos(static_cast<GLFWwindow*>(m_instance), static_cast<double>(position.x), static_cast<double>(position.y));
}

// Gets the cursor position
const Vector2 Input::GetCursorPos() {
    double x, y;
    glfwGetCursorPos(static_cast<GLFWwindow*>(m_instance), &x, &y);
    return { static_cast<float>(x), static_cast<float>(y) };
}

// Sets the instance of GLFWwindow
void Input::SetInstance(void *instance) {
    m_instance = instance;
}
