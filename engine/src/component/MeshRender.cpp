#include "MeshRender.h"
#include "Transform.h"
#include "../bufferManager.h"

MeshRender::MeshRender() : m_vertexArray(nullptr), m_indexEBO(-1), m_indexVBO(-1) {
    m_indexEBO = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO, 1024, sizeof(u32));
    m_indexVBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO, 1024, sizeof(Vertex3D));

    m_vertexArray = grr::gVertexArray::Create();
    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_indexEBO);
    grr::gVertexArray::Bind(m_indexVBO);

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, uv)));
    grr::gVertexArray::SetAttrib(3, 1, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, materialIndex)));

    m_vertexArray->unbind();
}

MeshRender::MeshRender(const Mesh &mesh) : m_outmesh(mesh), m_vertexArray(nullptr), m_indexEBO(-1), m_indexVBO(-1) {
    m_boundBox.min = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    m_boundBox.max = Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    
    for (const auto& vertex : mesh.m_vertices) {
        m_boundBox.min.x = std::min(m_boundBox.min.x, vertex.position.x);
        m_boundBox.min.y = std::min(m_boundBox.min.y, vertex.position.y);
        m_boundBox.min.z = std::min(m_boundBox.min.z, vertex.position.z);

        m_boundBox.max.x = std::max(m_boundBox.max.x, vertex.position.x);
        m_boundBox.max.y = std::max(m_boundBox.max.y, vertex.position.y);
        m_boundBox.max.z = std::max(m_boundBox.max.z, vertex.position.z);
    }

    m_indexEBO = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO, mesh.getindicecount(), sizeof(u32));
    m_indexVBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO, mesh.getverticecount(), sizeof(Vertex3D));

    m_vertexArray = grr::gVertexArray::Create();
    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_indexEBO);
    grr::gVertexArray::SetBufferUpdate(m_indexEBO, 0, mesh.getindicecount() * sizeof(u32), mesh.getindicedata());

    grr::gVertexArray::Bind(m_indexVBO);
    grr::gVertexArray::SetBufferUpdate(m_indexVBO, 0, mesh.getverticecount() * sizeof(Vertex3D), mesh.getverticedata());

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, uv)));
    grr::gVertexArray::SetAttrib(3, 1, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, materialIndex)));

    m_vertexArray->unbind();
}

MeshRender::~MeshRender() {
    if (m_vertexArray) {
        m_vertexArray->destroy();

        if (m_indexEBO) {
            grr::gVertexArray::DeleteBuffer(m_indexEBO);
        }

        if (m_indexVBO) {
            grr::gVertexArray::DeleteBuffer(m_indexVBO);
        }
    }
}

const BoundBox &MeshRender::getBoundBox() const {
    return m_boundBox;
}

Mesh &MeshRender::getMesh() {
    return m_outmesh;
}

void MeshRender::addMaterial(u32 index, const Material &material) {
    m_materials.emplace(index, material);
}

void MeshRender::buildMesh() {
    m_boundBox.min = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    m_boundBox.max = Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    
    for (const auto& vertex : m_outmesh.m_vertices) {
        m_boundBox.min.x = std::min(m_boundBox.min.x, vertex.position.x);
        m_boundBox.min.y = std::min(m_boundBox.min.y, vertex.position.y);
        m_boundBox.min.z = std::min(m_boundBox.min.z, vertex.position.z);

        m_boundBox.max.x = std::max(m_boundBox.max.x, vertex.position.x);
        m_boundBox.max.y = std::max(m_boundBox.max.y, vertex.position.y);
        m_boundBox.max.z = std::max(m_boundBox.max.z, vertex.position.z);
    }

    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_indexEBO);
    grr::gVertexArray::UpdateResize(m_indexEBO, m_outmesh.getindicecount(), sizeof(u32));
    grr::gVertexArray::SetBufferUpdate(m_indexEBO, 0, m_outmesh.getindicecount() * sizeof(u32), m_outmesh.getindicedata());

    grr::gVertexArray::Bind(m_indexVBO);
    grr::gVertexArray::UpdateResize(m_indexVBO, m_outmesh.getverticecount(), sizeof(Vertex3D));
    grr::gVertexArray::SetBufferUpdate(m_indexVBO, 0, m_outmesh.getverticecount() * sizeof(Vertex3D), m_outmesh.getverticedata());

    m_vertexArray->unbind();
}

void MeshRender::draw() {
    m_vertexArray->bind();
    
    grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_outmesh.getindicecount(), nullptr);
}


