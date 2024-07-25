#include "editorController.h"

#include "objectManager.h"

#include "component/camera.hpp"

#include "ui/ImGuiRender.h"

#include "settings.h"
#include "screen.h"
#include "input.h"
#include "timer.h"

float EditorController::movementSpeed = 30.0f;

EditorController::EditorController() : mouseSensitivity(10.5f), elapsed(Vector2::zero) {}

void EditorController::OnUpdate() {
    static ImGuiIO& io = ImGui::GetIO();

    if (!io.MouseDrawCursor || Settings::IsGameRunning) {
        return;
    }

    auto camera = Camera::Main();

    float velocity = movementSpeed * Time::deltaTime;

    Vector3 diretion;

    if (Input::IsKeyPress(Input::KEY_W)) {
        diretion += camera->getTransform()->getforward();
    }
    if (Input::IsKeyPress(Input::KEY_S)) {
        diretion -= camera->getTransform()->getforward();
    }
    if (Input::IsKeyPress(Input::KEY_A)) {
        diretion += camera->getTransform()->getright();
    }
    if (Input::IsKeyPress(Input::KEY_D)) {
        diretion -= camera->getTransform()->getright();
    }
    if (Input::IsKeyPress(Input::KEY_UP)) {
        diretion.y += 1.0f;
    }
    if (Input::IsKeyPress(Input::KEY_DOWN)) {
        diretion.y -= 1.0f;
    }

    if (diretion != Vector3::zero) {
        camera->getTransform()->position += Math::normalize(diretion) * velocity;
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
    auto camera = Camera::Main();
    // CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    Transform* m_transform = camera->getTransform();
    m_transform->rotation = Math::normalize(Math::euler({Math::rad(elapsed.y), Math::rad(-elapsed.x), 0.0f}));
}

void EditorController::processMouse(const Vector2& direction) {
    elapsed += direction * mouseSensitivity * Time::deltaTime;

    elapsed.y = std::max(std::min(elapsed.y, 89.0f), -89.0f);
    elapsed.x = std::fmod(elapsed.x, 360.0f);

    OnCameraUpdate();
}

