    #include "camera.hpp"

#include "transform.hpp"

#include "settings.h"

std::vector<Camera *> &(*Camera::GetCameras)();

Camera *(*Camera::Main)();

Camera *(*Camera::Current)();

Matrix4x4 Camera::getPerspesctive() const {
    return m_projection;
}

Matrix4x4 Camera::getViewmatrix() const {
    return Math::lookAt(m_transform->getPosition(), m_transform->getPosition() + m_transform->getForward(), m_transform->getUp());
}

Vector3 Camera::getPosition() const {
    return m_transform->getPosition();
}

Transform *Camera::getTransform() {
    return m_transform;
}

void Camera::setAspectRatio(float aspect) {
    if (aspect <= 0.0f) {
        return;
    }
    m_aspect = aspect;

    updateMatrix();
}

float Camera::getAspectRatio() {
    return m_aspect;
}

void Camera::lookIt(const Vector3 &direction) {
    m_transform->setLocalRotation(Math::normalize(Math::lookRotation(direction, Vector3::up)));
}

void Camera::updateMatrix() {
    m_projection = Math::perspective(Math::rad(m_fov), m_aspect, m_nearPlane, m_farPlane);
}
