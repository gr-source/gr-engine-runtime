#include "meshRender.hpp"
#include "meshFilter.hpp"
#include "transform.hpp"
#include "animator.hpp"

#include "../geometry/batching.hpp"
#include "../geometry/mesh.h"

MeshRender::MeshRender() : m_batched(nullptr) {}

MeshRender::~MeshRender() {
    if (!m_batched) {
        return;
    }
    m_batched->remove(&entity->getComponent<Transform>());
}

std::unordered_map<int, Material> &MeshRender::getMaterials() {
    return m_materials;
}

Material &MeshRender::addMaterial(int idx, const Material &material) {
    m_materials.emplace(idx, material);
    return m_materials[idx];
}

void MeshRender::build() {
    auto &meshFilter = entity->getComponent<MeshFilter>();
    auto mesh = meshFilter.getMesh();

    auto &transform = entity->getComponent<Transform>();

    std::string mName;
    for (auto &&[id, material] : m_materials) {
        mName += material.getName();
    }

    m_batched = Batching::Find(mName);
    if (m_batched) {
        m_batched->insert(&transform);
        return;
    }

    m_batched = &Batching::Add(mName, Batching{m_materials, mesh});
    m_batched->insert(&transform);
}

