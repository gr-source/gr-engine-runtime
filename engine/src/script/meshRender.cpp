#include "meshRender.hpp"
#include "meshFilter.hpp"
#include "transform.hpp"
#include "animator.hpp"

#include "geometry/batching.hpp"
#include "geometry/mesh.h"

#include "core/resources.h"

std::vector<Material> &MeshRender::getMaterials() {
    return m_materials;
}

void MeshRender::build() {
    auto &meshFilter = entity->getComponent<MeshFilter>();
    auto mesh = meshFilter.getMesh();
    if (!mesh) {
        throw std::runtime_error("Mesh pointer is null.");
    }

    auto &transform = entity->getComponent<Transform>();

    std::string mName;
    for (auto &material : m_materials) {
        mName += material.getName();
    }

    m_batched = Batching::Find(mName);
    if (m_batched) {
        m_batched->insert(&transform);
        return;
    }

    auto material = Resources::FindMaterial(mName);
    if (!material) {
        material = &Resources::AddMaterial(mName, m_materials);
    }

    m_batched = &Batching::Add(mName, material, mesh);
    m_batched->insert(&transform);
}
