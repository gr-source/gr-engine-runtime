#include "meshRender.hpp"
#include "meshFilter.hpp"
#include "animator.hpp"

#include "../geometry/mesh.h"

std::unordered_map<int, Material> &MeshRender::getMaterials() {
    return m_materials;
}

void MeshRender::draw() {
    m_vertexArray->bind();

    if (isIndice) {
        return grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_count, nullptr);
    }
    return grr::gVertexArray::DrawArrays(grr::PrimitiveType_Triangles, m_count);
}

Material &MeshRender::addMaterial(int idx, const Material &material) {
    m_materials.emplace(idx, material);
    return m_materials[idx];
}

void MeshRender::build() {
    auto& meshFilter = entity->getComponent<MeshFilter>();
    meshFilter.processBoundbox();

    auto mesh = meshFilter.getMesh();
    if (!mesh) {
        throw std::runtime_error("Invalid mesh: ");
    }

    if (!m_vertexArray) {
        m_vertexArray = grr::gVertexArray::Create();
    }
    
    if (!m_idxVBO) {
        m_idxVBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    }
    
    if (!m_idxEBO && mesh->hasIndices()) {
        isIndice = true;
        m_idxEBO = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO);
    }

    auto vertexCount = mesh->getVertices().size();
    auto vertexData = mesh->getVertices().data();

    m_count = vertexCount;

    m_vertexArray->bind();

    if (isIndice) {
        auto indiceCount = mesh->getIndices().size();
        auto indiceData = mesh->getIndices().data();

        m_count = indiceCount;

        grr::gVertexArray::Bind(m_idxEBO);
        grr::gVertexArray::UpdateResize(indiceCount * sizeof(grm::u32));
        grr::gVertexArray::SetBufferUpdate(0, indiceCount * sizeof(grm::u32), indiceData);
    }

    grr::gVertexArray::Bind(m_idxVBO);
    if (entity->hasComponent<Animator>()) {
        auto& boneweight = mesh->getBoneWeight();

        grr::gVertexArray::UpdateResize(vertexCount * sizeof(Vertex3D) + boneweight.size() * sizeof(BoneWeight));
        grr::gVertexArray::SetBufferUpdate(0, vertexCount * sizeof(Vertex3D), vertexData);
        grr::gVertexArray::SetBufferUpdate(vertexCount * sizeof(Vertex3D), boneweight.size() * sizeof(BoneWeight), boneweight.data());
    } else {
        grr::gVertexArray::UpdateResize(vertexCount * sizeof(Vertex3D));
        grr::gVertexArray::SetBufferUpdate(0, vertexCount * sizeof(Vertex3D), vertexData);
    }

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));
    grr::gVertexArray::SetAttrib(3, 1, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, mIdx)));

    if (entity->hasComponent<Animator>()) {
        grr::gVertexArray::SetAttribI(4, 4, sizeof(BoneWeight), reinterpret_cast<const void*>(vertexCount * sizeof(Vertex3D) + offsetof(BoneWeight, m_index)));
        grr::gVertexArray::SetAttrib(5, 4, sizeof(BoneWeight), reinterpret_cast<const void*>(vertexCount * sizeof(Vertex3D) + offsetof(BoneWeight, m_weight)));
    }
    m_vertexArray->unbind();
}
