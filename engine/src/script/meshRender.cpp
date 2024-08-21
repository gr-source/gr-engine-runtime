#include "meshRender.hpp"
#include "meshFilter.hpp"
#include "transform.hpp"
#include "animator.hpp"

#include "../geometry/batching.hpp"
#include "../geometry/mesh.h"

std::unordered_map<std::string, Batching> m_batched;

std::vector<GameObject *> MeshRender::s_objects;

MeshRender::MeshRender() : m_vertexArray(nullptr), m_idxVBO(0), m_idxEBO(0), m_count(0), isIndice(false) {}

MeshRender::~MeshRender() {
    if (m_vertexArray) {
        m_vertexArray->destroy();
    }

    if (m_idxEBO) {
        grr::gVertexArray::DeleteBuffer(m_idxEBO);
    }

    if (m_idxVBO) {
        grr::gVertexArray::DeleteBuffer(m_idxVBO);
    }

    auto it = std::find(s_objects.begin(), s_objects.end(), entity->getGameObject());
    if (it != s_objects.end()) {
        s_objects.erase(it);
    }
}

std::unordered_map<int, Material> &MeshRender::getMaterials() {
    return m_materials;
}

void MeshRender::Init() {
    s_objects.push_back(entity->getGameObject());
}

/*
void MeshRender::draw() {
    m_vertexArray->bind();

    if (isIndice) {
        return grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_count, nullptr);
    }
    return grr::gVertexArray::DrawArrays(grr::PrimitiveType_Triangles, m_count);
}
*/

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

    auto it = m_batched.find(mName);
    if (it != m_batched.end()) {
        it->second.insert(&transform, mesh);
        return;
    }

    auto &&[batched, name] = m_batched.emplace(mName, m_materials);
    (*batched).second.insert(&transform, mesh);
}

bool MeshRender::is = true;

void MeshRender::Draw() {
    if (is) {
        for (auto &&[name, batch] : m_batched) {
            batch.rebuild();
        }
        is = false;
    }
        
    for (auto &&[name, batch] : m_batched) {
        batch.draw();
    }

    // if (is) {
    //     is = false;
    // }
}
