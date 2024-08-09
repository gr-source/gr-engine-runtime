#pragma once

#include <gr-math/gmath.h>

#include <cstdint>
#include <cstring>

struct gHeader {
    char signature[4];
    float version;
};

struct gVertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
};

struct gMesh {
    char m_name[256];

    gVertex *m_vertices;
    grm::u32 m_verticeCount;

    grm::u32 *m_indices;
    grm::u32 m_indicesCount;
};

struct gModel {
    char m_name[256];

    Vector3 m_scale;
    Quaternion m_rotation;
    Vector3 m_position;
};


