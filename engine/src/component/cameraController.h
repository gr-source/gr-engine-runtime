#pragma once

#include "../ecs/Entity.h"

class CameraController : public Component {
public:
    CameraController();

    void OnInit() override;

    void OnStart() override;

    void OnUpdate() override;
    Vector3 up;

    Vector3 right;

    Vector3 forward;
protected:
    Transform* m_transform;


    float pitch;

    float yaw;

    void updateCursor();

    void OnCameraUpdate();

    void processMouse(const Vector2& direction);
};


