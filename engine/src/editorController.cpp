#include "editorController.h"

#include "objectManager.h"

#include "component/cameraComponent.h"

#include "ecs/EntityManager.h"

#include "ui/ImGuiRender.h"

#include "settings.h"
#include "screen.h"
#include "input.h"
#include "timer.h"

#include <GLFW/glfw3.h>

EditorController::EditorController() : movementSpeed(30.0f), mouseSensitivity(10.5f), pitch(0.0f), yaw(0.0f) {}

void EditorController::OnInit() {
}

void EditorController::OnUpdate() {
    static ImGuiIO& io = ImGui::GetIO();

    if (!io.MouseDrawCursor || Settings::IsGameRunning) {
        return;
    }
    if (CameraComponent::CameraID != 0) {
        CameraComponent::CameraID = 0;
    }

    CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    float velocity = movementSpeed * Time::deltaTime;

    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W)) {
        m_cameraComponent.gettransform()->position += m_cameraComponent.gettransform()->getforward() * velocity;
    }
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S)) {
        m_cameraComponent.gettransform()->position -= m_cameraComponent.gettransform()->getforward() * velocity;
    }
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A)) {
        m_cameraComponent.gettransform()->position += m_cameraComponent.gettransform()->getright() * velocity;
    }
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D)) {
        m_cameraComponent.gettransform()->position -= m_cameraComponent.gettransform()->getright() * velocity;
    }
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_UP)) {// up
        m_cameraComponent.gettransform()->position.y += velocity;
    }
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_DOWN)) {//down
        m_cameraComponent.gettransform()->position.y -= velocity;
    }

    updateCursor();
}

void EditorController::updateCursor() {
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

void EditorController::OnCameraUpdate() {
    CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    Transform* m_transform = m_cameraComponent.gettransform();
    m_transform->rotation = Math::normalize(Quaternion::Euler({Math::rad(-pitch), Math::rad(-yaw), 0.0f}));
}

void EditorController::processMouse(const Vector2& direction) {
    const Vector2 sensi = direction * mouseSensitivity * Time::deltaTime;

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

