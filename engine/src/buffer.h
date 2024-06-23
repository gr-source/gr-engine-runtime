#pragma once

#include "geometry/mesh.h"

#include <gr-render/gVertexArray.h>

class Buffer {
public:
    Buffer();
    ~Buffer();

    void buildMesh(const Mesh& mesh);

    void drawMesh();
private:
    grr::gVertexArray* m_vertexArray;

    u32 m_vbo;

    u32 m_ebo;

    u32 m_count;
};


