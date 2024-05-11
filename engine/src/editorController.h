#pragma once

#include "Global.h"

#include "common.h"

class EditorController {
public:
    float movementSpeed;
    float mouseSensitivity;

    EditorController();
    
    void OnInit();
    void OnUpdate();

    void updateCursor();
private:
    void processMouse(const Vector2& direction);

    Vector2 lastMousePos;

    GameObject* m_mainCamera;

    float pitch;
    float yaw;

    void OnCameraUpdate();

};

