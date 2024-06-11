#include "MeshRender.h"
#include "Transform.h"
#include "../bufferManager.h"

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

void MeshRender::draw() {
    m_vertexArray->bind();
    
    grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_outmesh.getindicecount(), nullptr);
}

/*
void MeshRender::OnInit() {
    std::hash<std::string> hashFunc;
    std::size_t hash{0};

    for (const auto& material : m_materials) {
        hash ^= hashFunc(material.second.getname());
    }

    auto it = BatchedGeometry::m_batchins.find(hash);
    if (it != BatchedGeometry::m_batchins.end()) {
        auto& batch = BatchedGeometry::m_batchins[hash];
        batch->add(entity->m_gameobject);
        return;
    }

    u32 verticeCount = 0;

    for (const auto& mesh : m_meshes) {
        const auto& vertices = mesh.getvertices();
        const auto& indices = mesh.getindices();

        m_outmesh.m_vertices.insert(m_outmesh.m_vertices.end(), vertices.begin(), vertices.end());

        for (const auto& index : indices) {
            m_outmesh.addindex(index + verticeCount);
        }

        verticeCount += vertices.size();
    }

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


    BatchedGeometry::m_batchins.emplace(hash, std::make_unique<BatchedGeometry>(m_outmesh));
    auto& batch = BatchedGeometry::m_batchins[hash];
    batch->m_materials.insert(m_materials.begin(), m_materials.end());

    batch->add(entity->m_gameobject);

    m_meshes.clear();
}
*/



