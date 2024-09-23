#include "script/meshRender.hpp"

#include "script/meshFilter.hpp"
#include "script/transform.hpp"
#include "script/animator.hpp"

#include <algorithm>
#include <iostream>

#include "ecs/gameObject.hpp"

#include "core/assetsManager.hpp"

MeshRender::MeshRender() : m_batched(nullptr), flags(Batching::Batching_Flags_Dynamic), m_transform(nullptr) {}

// TODO: Os scripts estao sendo deletados antes
MeshRender::~MeshRender() {
    if (!m_batched) {
        return;
    }
    m_batched->remove(m_transform, false);
}

void MeshRender::Init() {
    m_transform = &GetComponent<Transform>();
}

void MeshRender::build() {
}

void MeshRender::setMeshOptimize(int value) {
    flags = value;

    build();

    auto &transform = GetComponent<Transform>();
    for (auto &child : transform.getChildren()) {
        auto gameObject = child->getGameObject();

        if (!gameObject->hasComponent<MeshRender>()) {
            continue;
        }
        auto &meshRender = gameObject->getComponent<MeshRender>();
        meshRender.setMeshOptimize(value);
    }
}

void MeshRender::Enable() {
    if (!m_batched) {
        return;
    }
    m_batched->insert(&GetComponent<Transform>());
}

void MeshRender::Disable() {
    if (!m_batched) {
        return;
    }
    m_batched->remove(&GetComponent<Transform>(), false);
}

std::ostream &MeshRender::Serialize(std::ostream &os) {
    bool hasMaterial = !m_materials.empty();

    os.write(reinterpret_cast<const char *>(&hasMaterial), sizeof(bool));

    if (!hasMaterial) {
        return os;
    }

    std::string mName;
    for (auto &material : m_materials) {
        mName += material.getName();
    }

    auto nSize = mName.size();
    os.write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t));

    os.write(mName.data(), nSize);

    return os;
}

std::istream &MeshRender::Deserialize(std::istream &is) {
    bool hasMaterial;

    is.read(reinterpret_cast<char *>(&hasMaterial), sizeof(bool));

    if (!hasMaterial) {
        return is;
    }
    
    std::size_t nSize;
    is.read(reinterpret_cast<char *>(&nSize), sizeof(std::size_t));

    std::string name(nSize, '\0');
    is.read(name.data(), nSize);

    auto material = AssetsManager::FindMaterial(name);
    if (!material) {
        throw std::runtime_error(GetGameObject()->getName() + ": do not Deserialize MeshRender.");
    }
    m_materials = *material;

    build();

    return is;
}
