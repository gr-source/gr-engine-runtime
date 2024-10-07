#include "editorController.h"

#include "script/transform.hpp"
#include "script/camera.hpp"

#include "ui/ImGuiRender.h"

#include "settings.h"
#include "screen.h"
#include "input.h"
#include "timer.h"

Camera *EditorController::m_mainCamera = nullptr;
ImGuiIO *EditorController::m_io = nullptr;

float EditorController::movementSpeed = 30.0f;
float EditorController::mouseSensitivity = 0.1f;

Vector2 EditorController::lastMousePos = {0.0f, 0.0f};
Vector2 EditorController::elapsed = {0.0f, 0.0f};

void EditorController::Initialize() {
    m_io = ImGuiRender::GetIO();
    if (!m_io) {
        throw std::runtime_error("IO pointer do not is valid.");
    }
}

void EditorController::Update() {
    if (!Camera::Main && !Camera::Current) {
        return;
    }

    m_mainCamera = Camera::Main();

    if (!m_mainCamera || m_mainCamera != Camera::Current() || !m_io->MouseDrawCursor || Settings::IsGameRunning) {
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

    if (Math::magnitude(diretion) > 0.0f) {
        transform->addLocalPosition(Math::normalize(diretion) * movementSpeed * TimeLast::deltaTime);
    }

    updateCursor();
}

void EditorController::updateCursor() {
    auto cursorPos = Input::GetCursorPos();
    
    if (Input::IsMouseDown(MouseButton_Left) && lastMousePos == Vector2::zero) {
        lastMousePos = Input::GetCursorPos();
    }
    
    if (Input::IsMouseReleased(MouseButton_Left)) {
        lastMousePos = Vector2::zero;
    }

    if (Math::magnitude(lastMousePos) > 1e-5f) {
        auto delta = Input::GetCursorPos() - lastMousePos;

        ProcessMouse(delta);
            
        lastMousePos = Input::GetCursorPos();
    }
}

void EditorController::ProcessMouse(const Vector2 &delta) {
    elapsed += delta * mouseSensitivity;

    elapsed.y = std::clamp(elapsed.y, -89.0f, 89.0f);
    elapsed.x = std::fmod(elapsed.x, 360.0f);

    auto m_transform = m_mainCamera->getTransform();

    auto rotation = Math::euler({Math::rad(elapsed.y), Math::rad(-elapsed.x), 0.0f});

    rotation = Math::slerp(m_transform->getLocalRotation(), rotation, 25.0f * TimeLast::deltaTime);

    m_transform->setLocalRotation(Math::normalize(rotation + m_transform->getLocalRotation()));
}

