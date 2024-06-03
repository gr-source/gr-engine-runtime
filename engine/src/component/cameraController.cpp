#include "cameraController.h"
#include "cameraComponent.h"
#include "cameraComponent.h"
#include "Transform.h"

#include "../screen.h"
#include "../input.h"
#include "../timer.h"

CameraController::CameraController() : m_transform(nullptr), pitch(0.0f), yaw(0.0f) {}

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
<<<<<<< HEAD
    /*
=======
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
    static Vector2 lastMousePos = Input::GetCursorPos();
    const Vector2 delta = Input::GetCursorPos() - lastMousePos;

    if (Math::magnitude(delta) > 0.001f)
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

void CameraController::OnCameraUpdate() {
    m_transform->rotation = Math::normalize(Math::euler({Math::rad(-pitch), Math::rad(-yaw), 0.0f}));
}

void CameraController::processMouse(const Vector2& direction) {
    const Vector2 sensi = direction * 10.5f * Time::deltaTime;

    pitch -= sensi.y;
    yaw += sensi.x;

    // pitch = std::max(89.0f, std::min(-89.0f, pitch));

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

