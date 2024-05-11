#include "cameraController.h"
#include "cameraComponent.h"
#include "cameraComponent.h"
#include "Transform.h"

#include "../screen.h"
#include "../input.h"
#include "../timer.h"

#include <GLFW/glfw3.h>

CameraController::CameraController() : m_transform(nullptr), up(Vector3::up), right(Vector3::right), forward(Vector3::forward), pitch(0.0f), yaw(0.0f) {}

void CameraController::OnInit() {
    m_transform = &entity->getComponent<Transform>();
}

void CameraController::OnStart() {
    CameraComponent::CameraID = 1;
}

void CameraController::OnUpdate() {
    updateCursor();
}

void CameraController::updateCursor() {
    static Vector2 lastMousePos = Input::GetCursorPos();
    const Vector2 delta = Input::GetCursorPos() - lastMousePos;

    if (Math::magnitude(delta) > 0.00001f)
        processMouse(delta);

    lastMousePos = Screen::GetResolution() / 2.0f;

    if (!glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_MIDDLE)) {
        glfwSetCursorPos(glfwGetCurrentContext(), lastMousePos.x, lastMousePos.y);
        Input::SetCursorPos(lastMousePos.x, lastMousePos.y);
    }
}

void CameraController::OnCameraUpdate() {
    CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    forward = {
        std::cos(Math::rad(yaw)) * std::cos(Math::rad(pitch)),
        std::sin(Math::rad(pitch)),
        std::sin(Math::rad(yaw)) * std::cos(Math::rad(pitch))
    };

    forward = Math::normalize(forward);

    right = Math::normalize(Math::cross(forward, Vector3::up));
    Vector3 up = Math::normalize(Math::cross(right, forward));

    // m_cameraComponent.m_forward = forward;
    // m_cameraComponent.m_up = up;
}

void CameraController::processMouse(const Vector2& direction) {
    const Vector2 sensi = direction * 10.5f * Time::deltaTime;

    pitch -= sensi.y;
    yaw += sensi.x;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    if (yaw > 360.0f)
        yaw -= 360.0f;
    if (yaw < 0.0f)
        yaw += 360.0f;

    OnCameraUpdate();
}

