#pragma once

#include "../ecs/entity.h"

#include "../common.h"

class PlayerController : public Component {
public:
    PlayerController();

    void OnInit() override;

    void OnStart() override;

    void OnUpdate() override;

private:
    /* Trasnsform do local player */
    Transform* m_transform;

    /* Rigidbody do player */
    Rigidbody* m_rigidbody;

    /* Rigidbody do player */
    Animator* m_animator;

    Transform* m_camera;

    CameraController* m_cameraController;

    /* Velocidade de movimento */
    float pPlayerSpeed = 15.0f;

    /* Sensibiliddade */
    float mouseSensitivity = 10.5f;
};





