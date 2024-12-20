#include "meshRender.hpp"
#include "meshFilter.hpp"
#include "transform.hpp"
#include "animator.hpp"

#include "core/assetsManager.hpp"
#include "core/console.hpp"

#include "ecs/gameObject.hpp"

#include <core/geometry/mesh.hpp>

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

    // /*
    auto shader = AssetsManager::FindShader("dynamic_one_model");
    if (!shader) {
        return AssetsManager::LoadShader("dynamic_one_model",
            File::GetAsset("/shader/model/dynamic_model.frag"),
            File::GetAsset("/shader/model/dynamic_one_model.vert"),
            &MeshRender::SetupShader, this, mesh
        );
    }
    // */

    /*
    auto shader = AssetsManager::FindShader("dynamic_anim");
    if (!shader) {
        return AssetsManager::LoadShader("dynamic_anim",
            File::GetAsset("/shader/model/dynamic_model.frag"),
            File::GetAsset("/shader/model/dynamic_model_anim.vert"),
            &MeshRender::SetupShader, this, mesh
        );
    }
    */

    SetupShader(mesh, shader);
}

Batching *MeshRender::getBatchGroup() {
    return m_batched;
}

void MeshRender::SetupShader(Mesh *mesh, grr::gShader *shader) {
    m_batched = Batching::FindBatchByName(getGameObject()->getName());
    if (!m_batched) {
        m_batched = Batching::AddBatch(getGameObject()->getName(), &m_materials, shader, mesh->hasIndices() ? BatchingFlags_HasIndices : 0);
        m_batched->buildMeshe(mesh);
    }
    m_batched->insert(m_transform);
}




