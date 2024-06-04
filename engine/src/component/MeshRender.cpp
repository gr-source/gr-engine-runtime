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

    BatchedGeometry::m_batchins.emplace(hash, std::make_unique<BatchedGeometry>(99999));
    auto& batch = BatchedGeometry::m_batchins[hash];

    for (const auto& mesh : m_meshes) {
        const auto& vertices = mesh.getvertices();
        const auto& indices = mesh.getindices();

        batch->m_mesh.m_vertices.insert(batch->m_mesh.m_vertices.end(), vertices.begin(), vertices.end());

        for (const auto& index : indices) {
            batch->m_mesh.addindex(index + verticeCount);
        }

        verticeCount += vertices.size();
    }

    batch->add(entity->m_gameobject);
}
