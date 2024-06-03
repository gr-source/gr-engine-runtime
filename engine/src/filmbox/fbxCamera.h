#pragma once

#include <gr-math/gmath.h>

class FbxCamera {
public:
    FbxCamera(float fieldOfView = 45.0f, float farPlane = 0.1f, float nearPlane = 20.0f, float filmAspectRatio = 1.0f, const Vector3& up = Vector3::up, const Vector3& m_forward = Vector3::forward);

    const float& getfov() const;

    const float& getfarplane() const;

    const float& getnearplane() const;

    const float& getaspect() const;

    const Vector3& getup() const;

    const Vector3& getforward() const;
private:
    float m_fieldOfView;

    float m_farPlane;

    float m_nearPlane;

    float m_filmAspectRatio;

    Vector3 m_up;

    Vector3 m_forward;
};

