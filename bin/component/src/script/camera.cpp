#include "script/camera.hpp"
#include "script/transform.hpp"

#include "core/screen.hpp"

#include "ecs/gameObject.hpp"

#include <algorithm>

std::vector<Camera*> Camera::s_camera;

Camera *Camera::s_current = nullptr;

Camera *Camera::Main() {
    if (s_camera.empty()) {
        return s_current;
    }
    return s_camera[0];
}

Camera *Camera::Current() {
    return s_current;
}

Camera::Camera() : m_transform(nullptr), m_fov(45.0f), m_aspect(1.0f), m_farPlane(1000.0f), m_nearPlane(0.001f), m_projection(Matrix4x4::identityMatrix) {
    m_aspect = static_cast<float>(Screen::ScreenSize.x) / static_cast<float>(Screen::ScreenSize.y);
    
    updateProjection();
}

void Camera::Init() {
    m_transform = entity->getComponent<Transform>();

    s_camera.push_back(this);

    if (!s_current) {
        s_current = this;
    }
    m_aspect = static_cast<float>(Screen::ScreenSize.x) / static_cast<float>(Screen::ScreenSize.y);

    updateProjection();
}

void Camera::Destroy() {
    if (this == s_current) {
        s_current = nullptr;
    }
    s_camera.erase(std::remove(s_camera.begin(), s_camera.end(), this), s_camera.end());
}

void Camera::setEnable() {
    s_current = this;
}

Vector3 Camera::getPosition() const {
    return m_transform->getPosition();
}

void Camera::Enable() {
    s_camera.push_back(this);

    if (!s_current) {
        s_current = this;
    }
    m_aspect = static_cast<float>(Screen::ScreenSize.x) / static_cast<float>(Screen::ScreenSize.y);
    updateProjection();
}

void Camera::Disable() {
    if (this == s_current) {
        s_current = nullptr;
    }
    s_camera.erase(std::remove(s_camera.begin(), s_camera.end(), this), s_camera.end());
}

void Camera::updateProjection() {
    m_projection = Math::perspective(Math::rad(m_fov), m_aspect, m_nearPlane, m_farPlane);
}



