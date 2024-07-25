#pragma once

#include "../ecs/entity.h"

#include "../common.h"

class CameraController : public Component {
public:
    CameraController();

    void Init() override;

    void Start() override;

    void Update() override;

protected:
    Transform* m_transform;

    float pitch;

    float yaw;

    void updateCursor();

    void OnCameraUpdate();

    void processMouse(const Vector2& direction);
};


