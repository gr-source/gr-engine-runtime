#pragma once

#include "Global.h"

#include "common.h"

class EditorController {
public:
    float movementSpeed;
    float mouseSensitivity;

    EditorController();
    
    void OnUpdate();

    void updateCursor();
private:
    void processMouse(const Vector2& direction);

    Vector2 lastMousePos;

    GameObject* m_mainCamera;

    Vector2 elapsed;

    void OnCameraUpdate();

};

