#pragma once

#include "../ecs/component.h"

#include "default.hpp"

class Camera : public Component {
public:
    static std::vector<Camera *> &(*GetCameras)();

    static void (*SetEnable)(Camera *camera);

    static Camera *Main();

    static Camera *Current();

    Camera();

    const Matrix4x4 getPerspesctive() const;

    const Matrix4x4 getViewmatrix() const;

    const Vector3 getPosition() const;

    Transform *getTransform();

    void setAspectRatio(float aspect);

private:
    Transform *m_transform;

    float m_fov;
    float m_aspect;
    float m_farPlane;
    float m_nearPlane;
};
