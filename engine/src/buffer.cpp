#include "buffer.h"

#include "geometry/vertex3D.h"

Buffer::Buffer() : m_vertexArray(grr::gVertexArray::Create()), m_vbo(grr::gVertexArray::CreateBuffer(grr::BufferType_VBO)), m_ebo(grr::gVertexArray::CreateBuffer(grr::BufferType_EBO)), m_count(0) {
    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_ebo);

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));

    m_vertexArray->unbind();
}

Buffer::~Buffer() {
    if (m_vertexArray) {
        m_vertexArray->destroy();
    }

    if (m_ebo) {
        grr::gVertexArray::DeleteBuffer(m_ebo);
    }

    if (m_vbo) {
        grr::gVertexArray::DeleteBuffer(m_vbo);
    }
}

void Buffer::buildMesh(const Mesh &mesh) {
    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_ebo);
    grr::gVertexArray::UpdateResize(mesh.getIndiceCount(), sizeof(u32));
    grr::gVertexArray::SetBufferUpdate(0, mesh.getIndiceCount() * sizeof(u32), mesh.getIndiceData());

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResize(mesh.getVerticeCount(), sizeof(Vertex3D));
    grr::gVertexArray::SetBufferUpdate(0, mesh.getVerticeCount() * sizeof(Vertex3D), mesh.getVerticeData());

    m_vertexArray->unbind();

    m_count = mesh.getIndiceCount();
}

void Buffer::drawMesh() {
    if (!m_count) return;

    m_vertexArray->bind();

    grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_count, nullptr);

    m_vertexArray->unbind();
}
