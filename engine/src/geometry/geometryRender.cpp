#include "geometryRender.h"

// #include "../bufferManager.h"

GeometryRender::GeometryRender(const Mesh &mesh) : m_indexEBO(-1), m_indexVBO(-1), m_indexINS(-1) {
    m_vertexArray = grr::gVertexArray::Create();
    m_vertexArray->bind();

    m_indexEBO = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO);
    if (m_indexEBO == -1) {
        throw std::runtime_error("Falied to create EBO Buffer.");
    }

    m_indexVBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    if (m_indexVBO == -1) {
        throw std::runtime_error("Falied to create VBO Buffer.");
    }

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), (const void*)offsetof(Vertex3D, position));

    // m_indexINS = grr::gVertexArray::CreateBuffer(grr::BufferType_DEFAULT, 1, sizeof(Matrix4x4));
    // if (m_indexINS == -1) {
    //     throw std::runtime_error("Falied to create INSTANCED Buffer.");
    // }

    m_vertexArray->unbind();
}

