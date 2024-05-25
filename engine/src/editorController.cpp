#include "editorController.h"

#include "objectManager.h"

#include "component/cameraComponent.h"

#include "ui/ImGuiRender.h"

#include "settings.h"
#include "screen.h"
#include "input.h"
#include "timer.h"

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

    Vector3 diretion;

    // if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W)) {
    if (Input::IsKeyPress(Input::KEY_W)) {
        diretion += m_cameraComponent.gettransform()->getforward();
    }
    if (Input::IsKeyPress(Input::KEY_S)) {
        diretion -= m_cameraComponent.gettransform()->getforward();
    }
    if (Input::IsKeyPress(Input::KEY_A)) {
        diretion += m_cameraComponent.gettransform()->getright();
    }
    if (Input::IsKeyPress(Input::KEY_D)) {
        diretion -= m_cameraComponent.gettransform()->getright();
    }
    if (Input::IsKeyPress(Input::KEY_UP)) {// up
        diretion.y += 1.0f;
    }
    if (Input::IsKeyPress(Input::KEY_DOWN)) {//down
        diretion.y -= 1.0f;
    }

    if (diretion != Vector3::zero) {
        m_cameraComponent.gettransform()->position += Math::normalize(diretion) * velocity;
    }

    updateCursor();
}

void EditorController::updateCursor() {
    static Vector2 lastMousePos = Input::GetCursorPos();
    const Vector2 delta = Input::GetCursorPos() - lastMousePos;

    if (Math::magnitude(delta) > 0.00001f)
        processMouse(delta);

    lastMousePos = Screen::GetResolution() / 2.0f;

    if (!Input::IsMousePress(Input::BUTTON_MIDDLE)) {
        Input::SetCursorPos(lastMousePos);
    }
}

void EditorController::OnCameraUpdate() {
    CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    Transform* m_transform = m_cameraComponent.gettransform();
    m_transform->rotation = Math::normalize(Math::euler({Math::rad(-pitch), Math::rad(-yaw), 0.0f}));
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

