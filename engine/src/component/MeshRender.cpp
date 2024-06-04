#include "MeshRender.h"
#include "meshFilter.h"

#include "Transform.h"

#include "../bufferManager.h"

std::unordered_map<std::size_t, bool> MeshRender::m_batchins;

MeshRender::MeshRender(const std::vector<Mesh> &meshes, const std::unordered_map<u32, Material>& materials) : m_materials(materials), m_transform(nullptr) {
    std::hash<std::string> m_hashName;

    std::size_t hash{0};

    for (const auto& m : materials) {
        hash ^= m_hashName(m.second.getname());
    }
    auto it = m_batchins.find(hash);
    if (it != m_batchins.end()) {
        std::cout << "Mesh added ind bacthin: " << meshes[0].getname() << std::endl;
    }
    
    m_batchins.emplace(hash, true);
}

void MeshRender::OnInit() {
    m_transform = &entity->getComponent<Transform>();
}

MeshRender::~MeshRender() {
    // gameStatics.vertices -= verticeCount;
    // gameStatics.indices  -= indiceCount;

    // BatchingManager::drawCount -= 1;
}
