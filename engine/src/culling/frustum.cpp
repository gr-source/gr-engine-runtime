#include "frustum.h"

enum FrustumSide {
	RIGHT	= 0,		// The RIGHT side of the frustum
	LEFT	= 1,		// The LEFT	 side of the frustum
	BOTTOM	= 2,		// The BOTTOM side of the frustum
	TOP		= 3,		// The TOP side of the frustum
	BACK	= 4,		// The BACK	side of the frustum
	FRONT	= 5			// The FRONT side of the frustum
}; 

enum PlaneData {
	A = 0,				// The X value of the plane's normal
	B = 1,				// The Y value of the plane's normal
	C = 2,				// The Z value of the plane's normal
	D = 3				// The distance the plane is from the origin
};

void NormalizePlane(float frustum[6][4], int side) {
	float magnitude = std::sqrt(frustum[side][A] * frustum[side][A] +  frustum[side][B] * frustum[side][B] + frustum[side][C] * frustum[side][C]);

	// Then we divide the plane's values by it's magnitude.
	// This makes it easier to work with.
	frustum[side][A] /= magnitude;
	frustum[side][B] /= magnitude;
	frustum[side][C] /= magnitude;
	frustum[side][D] /= magnitude; 
}

void Frustum::CalculateFrustum(const Matrix4x4& vp) {    
	// This will extract the RIGHT side of the frustum
	m_Frustum[RIGHT][A] = vp[ 3] - vp[ 0];
	m_Frustum[RIGHT][B] = vp[ 7] - vp[ 4];
	m_Frustum[RIGHT][C] = vp[11] - vp[ 8];
	m_Frustum[RIGHT][D] = vp[15] - vp[12];

	NormalizePlane(m_Frustum, RIGHT);

	// This will extract the LEFT side of the frustum
	m_Frustum[LEFT][A] = vp[ 3] + vp[ 0];
	m_Frustum[LEFT][B] = vp[ 7] + vp[ 4];
	m_Frustum[LEFT][C] = vp[11] + vp[ 8];
	m_Frustum[LEFT][D] = vp[15] + vp[12];

	// Normalize the LEFT side
	NormalizePlane(m_Frustum, LEFT);

	// This will extract the BOTTOM side of the frustum
	m_Frustum[BOTTOM][A] = vp[ 3] + vp[ 1];
	m_Frustum[BOTTOM][B] = vp[ 7] + vp[ 5];
	m_Frustum[BOTTOM][C] = vp[11] + vp[ 9];
	m_Frustum[BOTTOM][D] = vp[15] + vp[13];

	// Normalize the BOTTOM side
	NormalizePlane(m_Frustum, BOTTOM);

	// This will extract the TOP side of the frustum
	m_Frustum[TOP][A] = vp[ 3] - vp[ 1];
	m_Frustum[TOP][B] = vp[ 7] - vp[ 5];
	m_Frustum[TOP][C] = vp[11] - vp[ 9];
	m_Frustum[TOP][D] = vp[15] - vp[13];

	// Normalize the TOP side
	NormalizePlane(m_Frustum, TOP);

	// This will extract the BACK side of the frustum
	m_Frustum[BACK][A] = vp[ 3] - vp[ 2];
	m_Frustum[BACK][B] = vp[ 7] - vp[ 6];
	m_Frustum[BACK][C] = vp[11] - vp[10];
	m_Frustum[BACK][D] = vp[15] - vp[14];

	// Normalize the BACK side
	NormalizePlane(m_Frustum, BACK);

	// This will extract the FRONT side of the frustum
	m_Frustum[FRONT][A] = vp[ 3] + vp[ 2];
	m_Frustum[FRONT][B] = vp[ 7] + vp[ 6];
	m_Frustum[FRONT][C] = vp[11] + vp[10];
	m_Frustum[FRONT][D] = vp[15] + vp[14];

	// Normalize the FRONT side
	NormalizePlane(m_Frustum, FRONT);
}

bool Frustum::PointInFrustum(const Vector3& point) {
	for (int i = 0; i < 6; i++) {
		if (m_Frustum[i][A] * point.x + m_Frustum[i][B] * point.y + m_Frustum[i][C] * point.z + m_Frustum[i][D] <= 0) {
			return false;
		}
	}
	return true;
}

bool Frustum::SphereInFrustum(const Vector3& point, float radius) {
	for (int i = 0; i < 6; i++)	{
		if (m_Frustum[i][A] * point.x + m_Frustum[i][B] * point.y + m_Frustum[i][C] * point.z + m_Frustum[i][D] <= -radius) {
			return false;
		}
	}
	return true;
}

#include "../renderer.h"

const bool Frustum::IsCubeInFrustumAABB(const Vector3& min, const Vector3& max) const {
    Vector3 points[8] {
        Vector3(min.x, min.y, min.z),
        Vector3(max.x, min.y, min.z),
        Vector3(max.x, max.y, min.z),
        Vector3(min.x, max.y, min.z),
 
        Vector3(min.x, min.y, max.z),
        Vector3(max.x, min.y, max.z),
        Vector3(max.x, max.y, max.z),
        Vector3(min.x, max.y, max.z)
    };

	// Desenhar as linhas do cubo usando as coordenadas dos pontos do frustum
	/*
	DebugDraw::addLine(points[0], points[1], Color::blue);
	DebugDraw::addLine(points[1], points[2], Color::blue);
	DebugDraw::addLine(points[2], points[3], Color::blue);
	DebugDraw::addLine(points[3], points[0], Color::blue);

	DebugDraw::addLine(points[4], points[5], Color::blue);
	DebugDraw::addLine(points[5], points[6], Color::blue);
	DebugDraw::addLine(points[6], points[7], Color::blue);
	DebugDraw::addLine(points[7], points[4], Color::blue);

	DebugDraw::addLine(points[0], points[4], Color::blue);
	DebugDraw::addLine(points[1], points[5], Color::blue);
	DebugDraw::addLine(points[2], points[6], Color::blue);
	DebugDraw::addLine(points[3], points[7], Color::blue);
	*/

	for (int i = 0; i < 6; i++ ) {
		if (m_Frustum[i][A] * (points[0].x) + m_Frustum[i][B] * (points[0].y) + m_Frustum[i][C] * (points[0].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[1].x) + m_Frustum[i][B] * (points[1].y) + m_Frustum[i][C] * (points[1].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[2].x) + m_Frustum[i][B] * (points[2].y) + m_Frustum[i][C] * (points[2].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[3].x) + m_Frustum[i][B] * (points[3].y) + m_Frustum[i][C] * (points[3].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[4].x) + m_Frustum[i][B] * (points[4].y) + m_Frustum[i][C] * (points[4].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[5].x) + m_Frustum[i][B] * (points[5].y) + m_Frustum[i][C] * (points[5].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[6].x) + m_Frustum[i][B] * (points[6].y) + m_Frustum[i][C] * (points[6].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[7].x) + m_Frustum[i][B] * (points[7].y) + m_Frustum[i][C] * (points[7].z) + m_Frustum[i][D] > 0)
		   continue;
		return false;
	}
	return true;
}

const bool Frustum::IsCubeInFrustumOOBB(const Vector3 &min, const Vector3 &max, const Matrix4x4 &model) const {
    Vector3 points[8] {
        Vector3(model * Vector4(min.x, min.y, min.z, 1.0f)),
        Vector3(model * Vector4(max.x, min.y, min.z, 1.0f)),
        Vector3(model * Vector4(max.x, max.y, min.z, 1.0f)),
        Vector3(model * Vector4(min.x, max.y, min.z, 1.0f)),
 
        Vector3(model * Vector4(min.x, min.y, max.z, 1.0f)),
        Vector3(model * Vector4(max.x, min.y, max.z, 1.0f)),
        Vector3(model * Vector4(max.x, max.y, max.z, 1.0f)),
        Vector3(model * Vector4(min.x, max.y, max.z, 1.0f))
    };

	// Desenhar as linhas do cubo usando as coordenadas dos pontos do frustum
	DebugDraw::addLine(points[0], points[1], Color::blue);
	DebugDraw::addLine(points[1], points[2], Color::blue);
	DebugDraw::addLine(points[2], points[3], Color::blue);
	DebugDraw::addLine(points[3], points[0], Color::blue);

	DebugDraw::addLine(points[4], points[5], Color::blue);
	DebugDraw::addLine(points[5], points[6], Color::blue);
	DebugDraw::addLine(points[6], points[7], Color::blue);
	DebugDraw::addLine(points[7], points[4], Color::blue);

	DebugDraw::addLine(points[0], points[4], Color::blue);
	DebugDraw::addLine(points[1], points[5], Color::blue);
	DebugDraw::addLine(points[2], points[6], Color::blue);
	DebugDraw::addLine(points[3], points[7], Color::blue);

	for (int i = 0; i < 6; i++ ) {
		if (m_Frustum[i][A] * (points[0].x) + m_Frustum[i][B] * (points[0].y) + m_Frustum[i][C] * (points[0].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[1].x) + m_Frustum[i][B] * (points[1].y) + m_Frustum[i][C] * (points[1].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[2].x) + m_Frustum[i][B] * (points[2].y) + m_Frustum[i][C] * (points[2].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[3].x) + m_Frustum[i][B] * (points[3].y) + m_Frustum[i][C] * (points[3].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[4].x) + m_Frustum[i][B] * (points[4].y) + m_Frustum[i][C] * (points[4].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[5].x) + m_Frustum[i][B] * (points[5].y) + m_Frustum[i][C] * (points[5].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[6].x) + m_Frustum[i][B] * (points[6].y) + m_Frustum[i][C] * (points[6].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[7].x) + m_Frustum[i][B] * (points[7].y) + m_Frustum[i][C] * (points[7].z) + m_Frustum[i][D] > 0)
		   continue;
		return false;
	}
	return true;
}
