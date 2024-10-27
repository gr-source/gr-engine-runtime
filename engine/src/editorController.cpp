#include "editorController.h"

#include "script/transform.hpp"
#include "script/camera.hpp"

#include "ui/ImGuiRender.h"

#include "core/console.hpp"
#include "core/screen.hpp"
#include "core/input.hpp"
#include "core/timer.hpp"

#include "settings.h"

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
    Console::Message(LogInfo::ERROR, "Health: %f - %d", 15.5f, 90);
}

void EditorController::Update() {
    if (!Settings::bGameRunning && !Camera::Main && !Camera::Current) {
        return;
    }

    m_mainCamera = Camera::Main();

    if (!m_mainCamera || m_mainCamera != Camera::Current() || !m_io->MouseDrawCursor || Settings::bGameRunning) {
        return;
    }

    auto transform = m_mainCamera->getTransform();

    Vector3 diretion = Vector3::zero;

    if (Input::IsKeyPress(Input::InputKey_W)) {
        diretion += transform->getforward();
    }
    if (Input::IsKeyPress(Input::InputKey_S)) {
        diretion -= transform->getforward();
    }
    if (Input::IsKeyPress(Input::InputKey_A)) {
        diretion += transform->getright();
    }
    if (Input::IsKeyPress(Input::InputKey_D)) {
        diretion -= transform->getright();
    }
    // if (Input::IsKeyPress(Input::InputKey_UP)) {
    //     diretion.y += 1.0f;
    // }
    // if (Input::IsKeyPress(Input::InputKey_DOWN)) {
    //     diretion.y -= 1.0f;
    // }

    if (Math::magnitude(diretion) > 0.0f) {
        transform->addLocalPosition(Math::normalize(diretion) * movementSpeed * TimeLast::deltaTime);
    }

    UpdateCursor(transform);
}

void EditorController::LookAt(Transform *transform) {
    auto dir = Math::normalize(m_mainCamera->getPosition() - transform->getPosition());

    m_mainCamera->lookIt(dir);

    // elapsed = {
    //     -Math::eulerAngles(m_mainCamera->getTransform()->getRotation()).y,
    //     Math::eulerAngles(m_mainCamera->getTransform()->getRotation()).x
    // };
}

void EditorController::UpdateCursor(Transform *transform) {
    auto cursorPos = Input::GetCursorPos();
    
    if (Input::IsMouseDown(Input::InputKey_Mouse_Left) && lastMousePos == Vector2::zero) {
        lastMousePos = Input::GetCursorPos();
    }
    
    if (Input::IsMouseReleased(Input::InputKey_Mouse_Left)) {
        lastMousePos = Vector2::zero;
    }

    if (Math::magnitude(lastMousePos) > 1e-5f) {
        auto delta = Input::GetCursorPos() - lastMousePos;

        ProcessMouse(delta, transform);
        
        lastMousePos = Input::GetCursorPos();
    }
}

void EditorController::ProcessMouse(const Vector2 &delta, Transform *transform) {
    elapsed += delta * mouseSensitivity;

    elapsed.y = std::clamp(elapsed.y, -89.0f, 89.0f);
    elapsed.x = std::fmod(elapsed.x, 360.0f);

    auto rotation = Math::euler({Math::rad(elapsed.y), Math::rad(-elapsed.x), 0.0f});

    rotation = Math::slerp(transform->getLocalRotation(), rotation, 25.0f * TimeLast::deltaTime);

    transform->setLocalRotation(Math::normalize(rotation + transform->getLocalRotation()));
}

