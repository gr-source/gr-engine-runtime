#pragma once

#include "../common.h"

class EditorController {
public:
    static void Initialize();

    static void Update();

    static float s_camSpeed;
    static float s_camSensivity;
private:
    static dVector2 lastMousePos;

    static dVector2 elapsed;

    static void MouseMovment(Transform& transform);
};

