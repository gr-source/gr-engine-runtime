#include "MeshRender.h"
#include "Transform.h"
#include "../bufferManager.h"

MeshRender::MeshRender(const std::vector<Mesh> &meshes, const std::unordered_map<u32, Material>& materials)
    : m_meshes(meshes), m_materials(materials), m_transform(nullptr) {
}

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
}
