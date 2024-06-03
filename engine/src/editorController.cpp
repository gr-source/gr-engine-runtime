#include "editorController.h"

#include "objectManager.h"

#include "component/cameraComponent.h"

#include "ui/ImGuiRender.h"

#include "settings.h"
#include "screen.h"
#include "input.h"
#include "timer.h"

EditorController::EditorController() : movementSpeed(30.0f), mouseSensitivity(10.5f), elapsed(Vector2::zero) {}

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

<<<<<<< HEAD
    /*
=======
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
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
    if (Input::IsKeyPress(Input::KEY_UP)) {
        diretion.y += 1.0f;
    }
    if (Input::IsKeyPress(Input::KEY_DOWN)) {
        diretion.y -= 1.0f;
    }
<<<<<<< HEAD
    */
=======
>>>>>>> 7c9a1b2 (	new file:   .gitignore)

    if (diretion != Vector3::zero) {
        m_cameraComponent.gettransform()->position += Math::normalize(diretion) * velocity;
    }

    updateCursor();
}

void EditorController::updateCursor() {
<<<<<<< HEAD
    /*
    static Vector2 lastMousePos = Input::GetCursorPos();
    const Vector2 delta = Input::GetCursorPos() - lastMousePos;

    // if (Math::magnitude(delta) > 0.00001f)
=======
    static Vector2 lastMousePos = Input::GetCursorPos();
    const Vector2 delta = Input::GetCursorPos() - lastMousePos;

    if (Math::magnitude(delta) > 0.00001f)
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
        processMouse(delta);

    lastMousePos = Screen::GetResolution() / 2.0f;

    if (!Input::IsMousePress(Input::BUTTON_MIDDLE)) {
        Input::SetCursorPos(lastMousePos);
    }
<<<<<<< HEAD
    */
=======
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
}

void EditorController::OnCameraUpdate() {
    CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    Transform* m_transform = m_cameraComponent.gettransform();
    m_transform->rotation = Math::normalize(Math::euler({Math::rad(elapsed.y), Math::rad(-elapsed.x), 0.0f}));
}

void EditorController::processMouse(const Vector2& direction) {
    elapsed += direction * mouseSensitivity * Time::deltaTime;

    elapsed.y = std::max(std::min(elapsed.y, 89.0f), -89.0f);
    elapsed.x = std::fmod(elapsed.x, 360.0f);

    OnCameraUpdate();
}

