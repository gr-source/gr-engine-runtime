#include "editorController.h"

#include <gr-base/script/transform.hpp>
#include <gr-base/script/camera.hpp>

#include <gr-base/core/console.hpp>
#include <gr-base/core/clock.hpp>

#include <gr-base/ecs/gameObject.hpp>

#include "ui/ImGuiRender.h"

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
}

void EditorController::Update(Camera *camera) {
    m_mainCamera = camera;

    if (!m_mainCamera || !m_io->MouseDrawCursor || Settings::bGameRunning) {
        return;
    }

    auto transform = m_mainCamera->getGameObject()->getTransform();

    Vector3 diretion = Vector3::zero;

    if (Input::IsKeyPress(InputKey_W)) {
        diretion += transform->getForward();
    }
    if (Input::IsKeyPress(InputKey_S)) {
        diretion -= transform->getForward();
    }
    if (Input::IsKeyPress(InputKey_A)) {
        diretion += transform->getRight();
    }
    if (Input::IsKeyPress(InputKey_D)) {
        diretion -= transform->getRight();
    }
    // if (Input::IsKeyPress(Input::InputKey_UP)) {
    //     diretion.y += 1.0f;
    // }
    // if (Input::IsKeyPress(Input::InputKey_DOWN)) {
    //     diretion.y -= 1.0f;
    // }

    if (Math::magnitude(diretion) > 0.0f) {
        transform->addLocalPosition(Math::normalize(diretion) * movementSpeed * Clock::deltaTime);
    }

    UpdateCursor(transform);
}

void EditorController::LookAt(Transform *transform) {
    /*
    auto dir = Math::normalize(m_mainCamera->getPosition() - transform->getPosition());

    m_mainCamera->lookIt(dir);

    auto euler = Math::eulerAngles(m_mainCamera->getTransform()->getLocalRotation());

    elapsed = {
        -euler.y,
        euler.x
    };

    elapsed.y = std::clamp(elapsed.y, -89.0f, 89.0f);
    elapsed.x = std::fmod(elapsed.x, 360.0f);
    */
}

void EditorController::UpdateCursor(Transform *transform) {
    auto cursorPos = Input::GetCursorPos();
    
    if (Input::IsMouseDown(InputKey_Mouse_Left) && lastMousePos == Vector2::zero) {
        lastMousePos = Input::GetCursorPos();
    }
    
    if (Input::IsMouseReleased(InputKey_Mouse_Left)) {
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

    rotation = Math::slerp(transform->getLocalRotation(), rotation, 25.0f * Clock::deltaTime);

    transform->setLocalRotation(Math::normalize(rotation));
}

