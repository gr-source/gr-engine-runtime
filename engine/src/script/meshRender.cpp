#include "meshRender.hpp"
#include "meshFilter.hpp"
#include "transform.hpp"
#include "animator.hpp"

#include "geometry/mesh.h"

#include "core/assetsManager.hpp"
#include "core/console.hpp"

#include "ecs/gameObject.hpp"

std::vector<std::shared_ptr<Material>> &MeshRender::getMaterials() {
    return m_materials;
}

// TODO: falta orimizar e corrigir erros do material;
void MeshRender::build() {
    if (!m_transform) {
        m_transform = entity->getComponent<Transform>();
    }

    if (!entity->hasComponent<MeshFilter>()) {
        return Console::LogError("[ MeshRender ] MeshFilter component fo not found.");
    }
    
    auto meshFilter = entity->getComponent<MeshFilter>();
    
    auto mesh = meshFilter->getMesh();
    if (!mesh) {
        return Console::LogError("[ MeshRender ] Object do not has mesh.");
    }

    if (m_batched) {
        m_batched->remove(m_transform);
        m_batched = nullptr;
    }

    if (m_flags == Batching::BatchingType_Dynamic) {
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

    auto transform = entity->getComponent<Transform>();
    for (auto &child : transform->getChildren()) {
        auto gameObject = child->getGameObject();

        if (!gameObject->hasComponent<MeshRender>()) {
            continue;
        }
        auto meshRender = gameObject->getComponent<MeshRender>();
        meshRender->setMeshOptimize(value);
    }
}

const int &MeshRender::getFlags() const {
    return m_flags;
}

void MeshRender::SetupShader(Mesh *mesh, grr::gShader *shader) {
    m_batched = Batching::FindBatchByName(getGameObject()->getName());
    if (!m_batched) {
        m_batched = &Batching::AddBatch(getGameObject()->getName(), &m_materials, mesh, shader, Batching::BatchingType_Dynamic);
    }
    
    m_batched->insert(m_transform);
}
