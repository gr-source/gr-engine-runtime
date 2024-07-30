#pragma once

#include <gr-math/gmath.h>
#include <cstdint>

struct gHeader {
    char signature[4];
    uint32_t version;
    char name[256];
};

struct gMeshHeader {
    uint32_t count;
};

struct gVertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int mIdx;
};

struct gMesh {
    uint32_t m_verticeCount;
    gVertex *m_vertices;

    uint32_t m_indiceCount;
    uint32_t *m_indices;
};


