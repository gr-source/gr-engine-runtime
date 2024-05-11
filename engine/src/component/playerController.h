#pragma once

#include "../ecs/Component.h"

class PlayerController : public Component {
public:
    PlayerController();

    void OnInit() override;

    void OnUpdate() override;

    void updateCursor();

    void processMouse(const Vector2& directio);

private:
    /* Trasnsform do local player */
    Transform* m_transform;

    /* Rigidbody do player */
    Rigidbody* m_rigidbody;

    /* Rigidbody do player */
    Animator* m_animator;

    CameraController* m_cameraController;

    /* Velocidade de movimento */
    float pPlayerSpeed = 15.0f;

    /* Sensibiliddade */
    float mouseSensitivity = 10.5f;
};





