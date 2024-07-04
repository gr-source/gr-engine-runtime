#pragma once

#include "../ecs/entity.h"

#include "../common.h"

class Camera : public Component {
public:
    static Camera *GetCurrent();

    Camera();

    void OnInit() override;

    const Matrix4x4 getViewMatrix() const;

    const Matrix4x4 getPerspesctive() const;

    const Vector3& getPosition() const;

    void setAspectRation(float aspect);

private:
    static Camera* s_current;

    float m_fov;

    float m_nearPlane;

    float m_farPlane;

    float m_aspect;

    Transform* m_transform;

    friend class GameObject;
};

