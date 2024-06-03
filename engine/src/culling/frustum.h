#pragma once

#include <gr-math/gmath.h>

#include <array>

class Frustum {
public:
    void CalculateFrustum(const Matrix4x4& vp);

    bool PointInFrustum(const Vector3& point);

    bool SphereInFrustum(const Vector3& point, float radius);

    const bool IsCubeInFrustumAABB(const Vector3& min, const Vector3& max) const;

    const bool IsCubeInFrustumOOBB(const Vector3& min, const Vector3& max, const Matrix4x4& model) const;

private:
    float m_Frustum[6][4];
};