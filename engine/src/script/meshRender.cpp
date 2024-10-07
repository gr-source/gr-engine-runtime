#include "meshRender.hpp"
#include "meshFilter.hpp"
#include "transform.hpp"
#include "animator.hpp"

#include "geometry/mesh.h"

#include "core/assetsManager.hpp"

std::vector<std::shared_ptr<Material>> &MeshRender::getMaterials() {
    return m_materials;
}

// TODO: falta orimizar e corrigir erros do material;
void MeshRender::build() {
    if (!entity->hasComponent<MeshFilter>()) {
        throw std::runtime_error("MeshFilter component do not found.");
    }
    
    auto &meshFilter = entity->getComponent<MeshFilter>();
    
    auto mesh = meshFilter.getMesh();
    if (!mesh) {
        throw std::runtime_error("Object do not has mesh.");
    }

    if (m_batched) {
        m_batched->remove(m_transform);
        m_batched = nullptr;
    }

    if (m_flags == Batching::Batching_Flags_Dynamic) {
        auto shader = AssetsManager::FindShader("dynamic_one_model");
        if (!shader) {
            AssetsManager::LoadShader("dynamic_one_model",
                File::GetAsset("/shader/model/dynamic_model.frag"),
                File::GetAsset("/shader/model/dynamic_one_model.vert"),
                &MeshRender::SetupShader, this, mesh
            );
            return;
        }

        SetupShader(mesh, shader);
        return;
    }
}

Batching *MeshRender::getBatchGroup() {
    return m_batched;
}

void MeshRender::setMeshOptimize(int value) {
    m_flags = value;

    build();

    auto &transform = entity->getComponent<Transform>();
    for (auto &child : transform.getChildren()) {
        auto gameObject = child->getGameObject();

        if (!gameObject->hasComponent<MeshRender>()) {
            continue;
        }
        auto &meshRender = gameObject->getComponent<MeshRender>();
        meshRender.setMeshOptimize(value);
    }
}

const int &MeshRender::getFlags() const {
    return m_flags;
}

void MeshRender::SetupShader(Mesh *mesh, grr::gShader *shader) {
    m_batched = Batching::Find(getGameObject()->getName());
    if (!m_batched) {
        m_batched = &Batching::Add(getGameObject()->getName(), &m_materials, mesh, shader, Batching::Batching_Flags_Dynamic);
    }
    
    m_batched->insert(m_transform);
}
