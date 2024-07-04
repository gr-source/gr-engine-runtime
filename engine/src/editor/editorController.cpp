#include "editorController.hpp"

#include "../component/camera.h"
#include "../gameObject.h"
#include "../screen.h"
#include "../input.h"
#include "../timer.h"

float EditorController::s_camSpeed = 10.0f;
float EditorController::s_camSensivity = 10.0f;

dVector2 EditorController::lastMousePos = {0, 0};//Vector2::zero;

dVector2 EditorController::elapsed = {0, 0};

void EditorController::Initialize() {
}

void EditorController::Update() {
    auto camera = Camera::GetCurrent();
    if (!camera) {
        return;
    }

    auto gameObject = Component::GetGameObject(camera); 
    if (!gameObject) {
        return;
    }

    auto& transform = gameObject->getTransform();

    MouseMovment(transform);

    Vector3 direction = Vector3::zero;
    if (Input::IsKeyPress(Input::KEY_W)) {
        direction += transform.getforward();
    }

    if (Input::IsKeyPress(Input::KEY_S)) {
        direction -= transform.getforward();
    }

    if (Input::IsKeyPress(Input::KEY_A)) {
        direction += transform.getright();
    }

    if (Input::IsKeyPress(Input::KEY_D)) {
        direction -= transform.getright();
    }

    if (Input::IsKeyPress(Input::KEY_UP)) {
        direction.y += 1.0f;
    }
    if (Input::IsKeyPress(Input::KEY_DOWN)) {
        direction.y -= 1.0f;
    }


    if (direction != Vector3::zero) {
        transform.position += Math::normalize(direction * s_camSpeed * Time::deltaTime);
    }
}

void EditorController::MouseMovment(Transform &transform) {
    auto afterMousePos = Input::GetCursorPos();

    auto direction = afterMousePos - lastMousePos;
    if (Math::Magnitude(direction) > 0.001f) {
        elapsed += direction * s_camSensivity * Time::deltaTime;

        elapsed.y = std::max(std::min(elapsed.y, 89.0), -89.0);
        elapsed.x = std::fmod(elapsed.x, 360.0);

        transform.rotation = Math::normalize(Math::euler({Math::rad(elapsed.y), Math::rad(-elapsed.x), 0.0f}));
    }

    auto screenSize = Screen::GetResolution() / 2;

    lastMousePos = {double(screenSize.x), double(screenSize.y)};

    if (!Input::IsMousePress(Input::BUTTON_MIDDLE)) {
        Input::SetCursorPos(lastMousePos);
    }
    
}

