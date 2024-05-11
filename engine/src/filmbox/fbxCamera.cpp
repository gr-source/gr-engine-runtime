#include "fbxCamera.h"

FbxCamera::FbxCamera(float fieldOfView, float farPlane, float nearPlane, float filmAspectRatio, const Vector3 &up, const Vector3 &forward) : m_fieldOfView(fieldOfView), m_farPlane(farPlane), m_nearPlane(nearPlane), m_filmAspectRatio(filmAspectRatio), m_up(up), m_forward(forward) {
}

const float &FbxCamera::getfov() const {
    return m_fieldOfView;
}

const float &FbxCamera::getfarplane() const {
    return m_farPlane;
}

const float &FbxCamera::getnearplane() const {
    return m_nearPlane;
}

const float &FbxCamera::getaspect() const {
    return m_filmAspectRatio;
}

const Vector3 &FbxCamera::getup() const {
    return m_up;
}

const Vector3 &FbxCamera::getforward() const {
    return m_forward;
}
