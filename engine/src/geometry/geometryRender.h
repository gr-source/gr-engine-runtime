#pragma once

#include "mesh.h"

#include <gr-render/gVertexArray.h>

class GeometryRender {
public:
    GeometryRender(const Mesh& mesh);

private:
    grr::gVertexArray* m_vertexArray;

    u32 m_indexEBO;
    u32 m_indexVBO;
    u32 m_indexINS;
    // std::unique_ptr<BufferManager> m_bufferManager;
};


