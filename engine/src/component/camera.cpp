#include "camera.h"
#include "Transform.h"

#include "../gameObject.h"
#include "../screen.h"

Camera Camera::s_main;

Camera* Camera::s_current = nullptr;

Camera *Camera::GetCurrent() {
    if (s_current != nullptr && s_current->gameObject->getActive()) {
        return s_current;
    }
    return nullptr;
}

Camera &Camera::GetMain() {
    return s_main;
}

Camera::Camera() : m_transform(nullptr), m_fov(60.0f), m_nearPlane(0.001f), m_farPlane(1000.0f), m_aspect(Screen::width / Screen::height) {}

void Camera::OnInit() {
    m_transform = &entity->getComponent<Transform>();
}

const Matrix4x4 Camera::getViewMatrix() const {
    return Math::lookAt(m_transform->getposition(), m_transform->getposition() + m_transform->getforward(), m_transform->getup());
}

const Matrix4x4 Camera::getPerspesctive() const {
    return Math::perspective(Math::rad(m_fov), m_aspect, m_farPlane, m_nearPlane);
}

const Vector3 &Camera::getPosition() const {
    return m_transform->position;
}
