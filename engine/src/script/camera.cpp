#include "camera.hpp"

#include "transform.hpp"

#include "../scriptManager/scriptManager.hpp"

std::vector<Camera *> &(*Camera::GetCameras)();

void (*Camera::SetEnable)(Camera *camera);

Camera *(*Camera::Main)();

Camera *(*Camera::Current)();

const Matrix4x4 Camera::getPerspesctive() const {
    return m_projection;
}

const Matrix4x4 Camera::getViewmatrix() const {
    return Math::lookAt(m_transform->getPosition(), m_transform->getPosition() + m_transform->getforward(), m_transform->getup());
}

const Vector3 Camera::getPosition() const {
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

void Camera::updateMatrix() {
    m_projection = Math::perspective(Math::rad(m_fov), m_aspect, m_farPlane, m_nearPlane);
}
