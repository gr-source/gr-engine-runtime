#include "meshRender.hpp"
#include "meshFilter.hpp"
#include "transform.hpp"
#include "animator.hpp"

#include "geometry/mesh.h"

#include "core/assetsManager.hpp"

std::vector<Material> &MeshRender::getMaterials() {
    return m_materials;
}

// TODO: falta orimizar e corrigir erros do material;
void MeshRender::build() {
    auto &meshFilter = entity->getComponent<MeshFilter>();
    auto mesh = meshFilter.getMesh();
    if (!mesh) {
        return;
        // throw std::runtime_error("Mesh or shader pointer is null.");
    }

    auto &transform = entity->getComponent<Transform>();

    if (m_batched) {
        m_batched->remove(&entity->getComponent<Transform>());
    }

    if (flags == Batching::Batching_Flags_Dynamic) {
        auto shader = AssetsManager::FindShader("/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_model.frag/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_one_model.vert");
        if (!shader) {
            throw std::runtime_error("Shader do not found.");
        }

        std::string mName;
        for (auto &material : m_materials) {
            mName += material.getName();
        }

        auto material = AssetsManager::FindMaterial(mName);
        if (!material) {
            material = &AssetsManager::AddMaterial(mName, m_materials);
        }

        m_batched = &Batching::Add(getGameObject()->getName(), material, mesh, shader, Batching::Batching_Flags_Dynamic);
        m_batched->insert(&transform);
        return;
    }

    std::string mName;
    for (auto &material : m_materials) {
        mName += material.getName();
    }

    m_batched = Batching::Find(mName);
    if (m_batched) {
        return m_batched->insert(&transform);
    }

    auto material = AssetsManager::FindMaterial(mName);
    if (!material) {
        material = &AssetsManager::AddMaterial(mName, m_materials);
    }

    std::string name;
    switch (flags) {
    case Batching::Batching_Flags_Instanced_Static:
    case Batching::Batching_Flags_Instanced_Dynamic:
        name = "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_model.frag/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_multi_model.vert";
        break;
    case Batching::Batching_Flags_Static_Mesh:
        name = "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_model.frag/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/static_multi_model.vert";
        break;
    default:
        throw std::runtime_error(".");
        break;
    }

    auto shader = AssetsManager::FindShader(name);
    if (!shader) {
        throw std::runtime_error("Shader do not found.");
    }

    m_batched = &Batching::Add(mName, material, mesh, shader, flags);
    m_batched->insert(&transform);
}

Batching *MeshRender::getBatchGroup() {
    return m_batched;
}

void MeshRender::setMeshOptimize(int value) {
    flags = value;

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
