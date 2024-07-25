#include "camera.hpp"

#include "Transform.h"

#include "../scriptManager/scriptManager.hpp"

Camera *Camera::Main() {
    if (!ScriptManager::MainCamera) {
        return nullptr;
    }
    return ScriptManager::MainCamera();
}

Camera *Camera::Current() {
    return ScriptManager::CurrentCamera();
}

Camera::Camera() : m_transform(nullptr), m_fov(45.0f), m_farPlane(1000.0f), m_nearPlane(0.1f) { }

const Matrix4x4 Camera::getPerspesctive() const {
    return Math::perspective(Math::rad(m_fov), m_aspect, m_farPlane, m_nearPlane);
}

const Matrix4x4 Camera::getViewmatrix() const {
    return Math::lookAt(m_transform->getposition(), m_transform->getposition() + m_transform->getforward(), m_transform->getup());
}

const Vector3 Camera::getPosition() const {
    return m_transform->getposition();
}

Transform *Camera::getTransform() {
    return m_transform;
}
