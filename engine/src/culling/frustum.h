#pragma once

#include <gr-math/gmath.h>

#include <array>

class Frustum {
public:
    void CalculateFrustum(const Matrix4x4& vp);

    bool PointInFrustum(const Vector3& point);

    bool SphereInFrustum(const Vector3& point, float radius);

    bool IsFrustum(const Vector3& min, const Vector3& max);

    bool CubeInFrustum(const Vector3& point, const Vector3& size);

private:
    float m_Frustum[6][4];
};