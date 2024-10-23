#include "input.h"

#include <GLFW/glfw3.h>

#include "ui/ImGuiRender.h"

#define GET_INSTANCE_PRESS(key) glfwGetKey((GLFWwindow*)Input::m_instance, key);
#define GET_INSTANCE_MOUSE_BUTTON(key) glfwGetMouseButton((GLFWwindow*)Input::m_instance, key);
#define GET_INSTANCE_MOUSE_POS() double x, y; glfwGetCursorPos((GLFWwindow*)Input::m_instance, &x, &y); return {static_cast<float>(x), static_cast<float>(y)};
#define SET_INSTANCE_MOUSE_POS(pos) glfwSetCursorPos((GLFWwindow*)Input::m_instance, static_cast<float>(pos.x), static_cast<float>(pos.y))

ImGuiKey get_key_to_glfw(grm::u32 key) {
    switch (key) {
    case Input::InputKey_W:
        return ImGuiKey_W;
    case Input::InputKey_A:
        return ImGuiKey_A;
    case Input::InputKey_S:
        return ImGuiKey_S;
    case Input::InputKey_D:
        return ImGuiKey_D;
    // case Input::InputKey_SPACE:
    //     return ImGuiKey_Space;
    // case Input::InputKey_UP:
    //     return ImGuiKey_UpArrow;
    // case Input::InputKey_DOWN:
    //     return ImGuiKey_DownArrow;
    default:
        return ImGuiKey_0;
    }
}

std::unordered_map<MouseButton, bool> Input::m_mouseButtonState = {
    {MouseButton_Right, false},
    {MouseButton_Left, false},
};

Input::MouseData Input::m_mouseData;

std::string Input::getKeyName(Key key) {
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

bool Input::IsKeyDown(Key key) {
    return false;
}

bool Input::IsKeyUp(Key key) {
    return false;
}

bool Input::IsKeyPress(Key key) {
    return ImGui::IsKeyDown(get_key_to_glfw(key));//GET_INSTANCE_PRESS(get_key_to_glfw(key));
}

bool Input::IsMouseReleased(Key button) {
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

bool Input::IsMouseDown(Key button) {
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

void Input::Initialize() {
}

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
