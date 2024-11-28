#pragma once

#include "default.hpp"

#include <ecs/component.hpp>

class Camera : public Component {
public:
    static std::vector<Camera *> &(*GetCameras)();

    static void (*SetActive)(Camera *camera);

    static Camera *(*Main)();

    static Camera *(*Current)();

    Camera() = delete;

    Matrix4x4 getPerspesctive() const;

    Matrix4x4 getViewmatrix() const;

    Vector3 getPosition() const;

    Transform *getTransform();

    void setAspectRatio(float aspect);

    float getAspectRatio();

    void lookIt(const Vector3 &direction);

private:
    Transform *m_transform;

    Matrix4x4 m_projection;

    float m_nearPlane;
    
    float m_farPlane;
    
    float m_aspect;

    float m_fov;

    void updateMatrix();
};
