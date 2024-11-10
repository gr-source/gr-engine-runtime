#include "input.hpp"

#include <GLFW/glfw3.h>

#include "ui/ImGuiRender.h"

#define GET_INSTANCE_PRESS(key) glfwGetKey((GLFWwindow*)Input::m_instance, key);
#define GET_INSTANCE_MOUSE_BUTTON(key) glfwGetMouseButton((GLFWwindow*)Input::m_instance, key);
#define GET_INSTANCE_MOUSE_POS() double x, y; glfwGetCursorPos((GLFWwindow*)Input::m_instance, &x, &y); return {static_cast<float>(x), static_cast<float>(y)};
#define SET_INSTANCE_MOUSE_POS(pos) glfwSetCursorPos((GLFWwindow*)Input::m_instance, static_cast<float>(pos.x), static_cast<float>(pos.y))

std::unordered_map<InputKey, int> Input::m_keyMap = {
    {InputKey_A, ImGuiKey_A},
    {InputKey_B, ImGuiKey_B},
    {InputKey_C, ImGuiKey_C},
    {InputKey_D, ImGuiKey_D},
    {InputKey_E, ImGuiKey_E},
    {InputKey_F, ImGuiKey_F},
    {InputKey_G, ImGuiKey_G},
    {InputKey_H, ImGuiKey_H},
    {InputKey_I, ImGuiKey_I},
    {InputKey_J, ImGuiKey_J},
    {InputKey_K, ImGuiKey_K},
    {InputKey_L, ImGuiKey_L},
    {InputKey_M, ImGuiKey_M},
    {InputKey_N, ImGuiKey_N},
    {InputKey_O, ImGuiKey_O},
    {InputKey_P, ImGuiKey_P},
    {InputKey_Q, ImGuiKey_Q},
    {InputKey_R, ImGuiKey_R},
    {InputKey_S, ImGuiKey_S},
    {InputKey_T, ImGuiKey_T},
    {InputKey_U, ImGuiKey_U},
    {InputKey_V, ImGuiKey_V},
    {InputKey_W, ImGuiKey_W},
    {InputKey_X, ImGuiKey_X},
    {InputKey_Y, ImGuiKey_Y},
    {InputKey_Z, ImGuiKey_Z},
    {InputKey_Space, ImGuiKey_Space},
    {InputKey_Enter, ImGuiKey_Enter},
    {InputKey_Escape, ImGuiKey_Escape},
    {InputKey_Arrow_Up, ImGuiKey_UpArrow},
    {InputKey_Arrow_Down, ImGuiKey_DownArrow},
    {InputKey_Arrow_Left, ImGuiKey_LeftArrow},
    {InputKey_Arrow_Right, ImGuiKey_RightArrow},
    {InputKey_LeftCtrl, ImGuiKey_LeftCtrl},
    {InputKey_RightCtrl, ImGuiKey_RightCtrl},
    {InputKey_LeftShift, ImGuiKey_LeftShift},
    {InputKey_RightShift, ImGuiKey_RightShift},
    {InputKey_LeftAlt, ImGuiKey_LeftAlt},
    {InputKey_RightAlt, ImGuiKey_RightAlt},
    {InputKey_Mouse_Left, ImGuiKey_MouseLeft},
    {InputKey_Mouse_Right, ImGuiKey_MouseRight},
    {InputKey_Mouse_ButtonMiddle, ImGuiKey_MouseMiddle}
};

Input::MouseData Input::m_mouseData;

std::string Input::getKeyName(InputKey key) {
    #define GET_ENUM_NAME(e) case e: return #e;
    switch (key) {
    GET_ENUM_NAME(InputKey_A)
    GET_ENUM_NAME(InputKey_W)
    GET_ENUM_NAME(InputKey_S)
    GET_ENUM_NAME(InputKey_D)
    // GET_ENUM_NAME(KEY_SPACE)
    default:
        return "";
        // GET_ENUM_NAME(NONE)
    }
    #undef GET_ENUM_NAME
}

bool Input::IsKeyDown(InputKey key) {
    return false;
}

bool Input::IsKeyUp(InputKey key) {
    return false;
}

bool Input::IsKeyPress(InputKey key) {
    return ImGui::IsKeyDown((ImGuiKey )m_keyMap[key]);//GET_INSTANCE_PRESS(get_key_to_glfw(key));
}

bool Input::IsMouseReleased(InputKey button) {
    switch (button) {
    case InputKey_Mouse_Left:
        return ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    case InputKey_Mouse_Right:
        return ImGui::IsMouseReleased(ImGuiMouseButton_Right);
    case InputKey_Mouse_ButtonMiddle:
        return ImGui::IsMouseReleased(ImGuiMouseButton_Middle);
    default:
        return false;
    }
}

bool Input::IsMouseDown(InputKey button) {
    switch (button) {
    case InputKey_Mouse_Left:
        return ImGui::IsMouseDown(ImGuiMouseButton_Left);
    case InputKey_Mouse_Right:
        return ImGui::IsMouseDown(ImGuiMouseButton_Right);
    case InputKey_Mouse_ButtonMiddle:
        return ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    default:
        return false;
    }
}

void Input::SetCursorPos(const Vector2& position) {
    glfwSetCursorPos(glfwGetCurrentContext(), static_cast<double>(position.x), static_cast<double>(position.y));
}

Vector2 Input::GetCursorPos() {
    double x, y;
    glfwGetCursorPos(glfwGetCurrentContext(), &x, &y);
    return {static_cast<float>(x), static_cast<float>(y)};
}

void Input::Initialize() {}

void Input::Update() {
    // auto &io = ImGui::GetIO();
    // auto window = glfwGetCurrentContext();

    // // m_mouseButtonState[MouseButton_Right] = ImGui::IsMouseDown(ImGuiMouseButton_Right);//glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? true : false;
    // // m_mouseButtonState[MouseButton_Left]  = ImGui::IsMouseDown(ImGuiMouseButton_Left);//glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? true : false;

    // if (m_mouseData.want) {
    //     glfwSetCursorPos(window, static_cast<double>(m_mouseData.pos.x), static_cast<double>(m_mouseData.pos.y));
    //     m_mouseData.want = false;
    // }
    // double x, y;
    // glfwGetCursorPos(window, &x, &y);

    // m_mouseData.pos = {static_cast<float>(x), static_cast<float>(y)};
}
