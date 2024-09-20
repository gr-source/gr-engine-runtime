#pragma once

#include "Global.h"

#include "common.h"

class EditorController {
public:
    static float movementSpeed;
    
    static float mouseSensitivity;

    static void Initialize();

    static void Update();

    static void updateCursor();

private:
    static ImGuiIO *m_io;
    
    static Camera *m_mainCamera;

    static Vector2 lastMousePos;

    static Vector2 elapsed;

    static Vector2 start;

    static void ProcessMouse(const Vector2 &delta);
};



