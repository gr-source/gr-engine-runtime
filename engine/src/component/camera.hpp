#pragma once

#include "../ecs/component.h"

#include "default.hpp"

class Camera : public Component {
public:
    static Camera *Main();

    static Camera *Current();

    Camera();

    const Matrix4x4 getPerspesctive() const;

    const Matrix4x4 getViewmatrix() const;

    const Vector3 getPosition() const;

    Transform *getTransform();

private:
    Transform *m_transform;

    float m_fov;
    float m_aspect;
    float m_farPlane;
    float m_nearPlane;

    static std::vector<Camera*> s_camera;

    static Camera *s_current;
};
