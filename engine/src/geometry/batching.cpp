#include "batching.hpp"

#include "mesh.h"

#include "script/transform.hpp"

#include "core/statistics.hpp"

#include "renderManager.h"

#include <cstring>

bool Batching::bRebuild = false;

std::unordered_map<std::string, Batching> Batching::m_batched;

Batching::Batching(std::unordered_map<int, Material> &materials) : m_materials(materials), m_vertexArray(nullptr), m_count(0), m_VBO(0), m_EBO(0), bHasIndice(false) {}

void Batching::insert(Transform *transform, Mesh *mesh) {
    m_meshes.emplace_back(transform, mesh);
}

void Batching::draw() {
    if (!m_vertexArray) {
        return;
    }
    setupMaterial();

    m_vertexArray->bind();

    if (bHasIndice) {
        return grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_count, nullptr);
    }
    return grr::gVertexArray::DrawArrays(grr::PrimitiveType_Triangles, m_count);
}

void Batching::rebuild() {
    Mesh m_mesh;

    grm::size verticeCount = 0;
    for (auto &&[transform, mesh] : m_meshes) {
        for (auto &vertice : mesh->getVertices()) {
            m_mesh.addVertex(Vector3(transform->getGlobalMatrix() * Vector4(vertice.position, 1.0f)), vertice.normal, vertice.texCoord, vertice.mIdx);
        }

        for (auto &indice : mesh->getIndices()) {
            m_mesh.addIndice(verticeCount + indice);
        }
        verticeCount += mesh->getVertices().size();
    }

    compile(&m_mesh);
}

void Batching::Renderer() {
    if (bRebuild) {
        for (auto &&[name, batch] : m_batched) {
            batch.rebuild();
        }
        bRebuild = false;
    }

    Statistics::m_drawCallCount = 0; 
        
    for (auto &&[name, batch] : m_batched) {
        batch.draw();

        Statistics::m_drawCallCount++;
    }
}

Batching *Batching::Find(const std::string &name) {
    auto it = m_batched.find(name);
    if (it == m_batched.end()) {
        return nullptr;
    }
    return &(it->second);
}

void Batching::compile(Mesh *mesh) {
    assert(mesh);

    if (!m_vertexArray) {
        m_vertexArray = grr::gVertexArray::Create();
    }
    
    if (!m_VBO) {
        m_VBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    }
    
    if (!m_EBO && mesh->hasIndices()) {
        bHasIndice = true;
        m_EBO = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO);
    }

    auto vertexCount = mesh->getVertices().size();
    auto vertexData = mesh->getVertices().data();

    m_count = vertexCount;

    m_vertexArray->bind();

    if (bHasIndice) {
        auto indiceCount = mesh->getIndices().size();
        auto indiceData = mesh->getIndices().data();

        m_count = indiceCount;

        grr::gVertexArray::Bind(m_EBO);
        grr::gVertexArray::UpdateResize(indiceCount * sizeof(grm::u32));
        grr::gVertexArray::SetBufferUpdate(0, indiceCount * sizeof(grm::u32), indiceData);
    }

    grr::gVertexArray::Bind(m_VBO);
    /*
    if (entity->hasComponent<Animator>()) {
        auto& boneweight = mesh->getBoneWeight();

        grr::gVertexArray::UpdateResize(vertexCount * sizeof(Vertex3D) + boneweight.size() * sizeof(BoneWeight));
        grr::gVertexArray::SetBufferUpdate(0, vertexCount * sizeof(Vertex3D), vertexData);
        grr::gVertexArray::SetBufferUpdate(vertexCount * sizeof(Vertex3D), boneweight.size() * sizeof(BoneWeight), boneweight.data());
    } else {
    */
    grr::gVertexArray::UpdateResize(vertexCount * sizeof(Vertex3D));
    grr::gVertexArray::SetBufferUpdate(0, vertexCount * sizeof(Vertex3D), vertexData);
    // }

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));
    grr::gVertexArray::SetAttrib(3, 1, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, mIdx)));

    /*
    if (entity->hasComponent<Animator>()) {
        grr::gVertexArray::SetAttribI(4, 4, sizeof(BoneWeight), reinterpret_cast<const void*>(vertexCount * sizeof(Vertex3D) + offsetof(BoneWeight, m_index)));
        grr::gVertexArray::SetAttrib(5, 4, sizeof(BoneWeight), reinterpret_cast<const void*>(vertexCount * sizeof(Vertex3D) + offsetof(BoneWeight, m_weight)));
    }
    */
    m_vertexArray->unbind();
}

void Batching::setupMaterial() {
    int idx[10]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idxEn[10]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (auto&& [index, material] : m_materials) {
        std::memset(idxEn, 10, 0);

        for (auto i=0;i<10;++i) {
            auto texture = material.getTexture(i);
            if (!texture || !texture->isValid()) {
                continue;
            }
            grr::gTexture::Bind(texture, idx[i]);
            idxEn[i] = 1;
        }

        grr::gShader::SetUniform<int>("m_texture", 10, (const int *)idx);
        grr::gShader::SetUniform<int>("m_activate", 10, (const int *)idxEn);

        grr::gShader::SetUniform("m_material[" + std::to_string(index) + "].albedo", material.getAlbedo());
        grr::gShader::SetUniform("m_material[" + std::to_string(index) + "].params", Vector3(material.getRoughness(), material.getMetallic(), material.getAmbientOcclusion()));
    }
}
