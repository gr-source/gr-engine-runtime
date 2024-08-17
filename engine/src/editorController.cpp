#include "editorController.h"

#include "component/transform.hpp"
#include "component/camera.hpp"

#include "ui/ImGuiRender.h"

#include "settings.h"
#include "input.h"
#include "timer.h"

Camera *EditorController::m_mainCamera = nullptr;

float EditorController::movementSpeed = 30.0f;
float EditorController::mouseSensitivity = 0.1f;

Vector2 EditorController::lastMousePos = {0.0f, 0.0f};
Vector2 EditorController::elapsed = {0.0f, 0.0f};

void EditorController::Initialize() {
    lastMousePos = Input::GetCursorPos();
}

void EditorController::Update() {
    static ImGuiIO& io = ImGui::GetIO();
    m_mainCamera = Camera::Main();

    if (m_mainCamera != Camera::Current() || !io.MouseDrawCursor || Settings::IsGameRunning) {
        return;
    }

    auto transform = m_mainCamera->getTransform();

    Vector3 diretion;

    if (Input::IsKeyPress(Input::KEY_W)) {
        diretion += transform->getforward();
    }
    if (Input::IsKeyPress(Input::KEY_S)) {
        diretion -= transform->getforward();
    }
    if (Input::IsKeyPress(Input::KEY_A)) {
        diretion += transform->getright();
    }
    if (Input::IsKeyPress(Input::KEY_D)) {
        diretion -= transform->getright();
    }
    if (Input::IsKeyPress(Input::KEY_UP)) {
        diretion.y += 1.0f;
    }
    if (Input::IsKeyPress(Input::KEY_DOWN)) {
        diretion.y -= 1.0f;
    }

    if (diretion != Vector3::zero) {
        transform->addLocalPosition(Math::normalize(diretion) * movementSpeed * Time::deltaTime);
    }

    updateCursor();
}

void EditorController::updateCursor() {
    const Vector2 delta = Input::GetCursorPos() - lastMousePos;

    // if (Math::magnitude(delta) > 1.0f)
        ProcessMouse(delta);

    auto center = Settings::ViewportSize / 2;
    lastMousePos = {static_cast<float>(center.x), static_cast<float>(center.y)};

    if (!Input::IsMousePress(Input::BUTTON_MIDDLE)) {
        Input::SetCursorPos(lastMousePos);
    }
}

void EditorController::ProcessMouse(const Vector2 &delta) {
    elapsed += delta * mouseSensitivity;

    elapsed.y = std::clamp(elapsed.y, -89.0f, 89.0f);
    elapsed.x = std::fmod(elapsed.x, 360.0f);

    auto m_transform = m_mainCamera->getTransform();

    auto rotation = Math::euler({Math::rad(elapsed.y), Math::rad(-elapsed.x), 0.0f});

    rotation = Math::slerp(m_transform->getLocalRotation(), rotation, 25.0f * Time::deltaTime);

    m_transform->setLocalRotation(Math::normalize(rotation));
}

