#include "batching.hpp"

#include "mesh.h"

#include "script/transform.hpp"

#include "core/statistics.hpp"

#include "renderManager.h"

#include <cstring>

#include "core/event.hpp"

std::unordered_map<std::string, std::unique_ptr<Batching>> Batching::m_batched;

Batching::Batching(const std::string &name, std::unordered_map<int, Material> &materials, Mesh *mesh) : m_name(name), m_materials(materials), m_vertexArray(nullptr), m_indiceCount(0), m_primCount(0), m_VBO(0), m_EBO(0), m_PBO(0), bHasIndice(false), isCompiled(false) {
    compileMesh(mesh);
}

Batching::~Batching() {
    if (m_vertexArray) {
        m_vertexArray->destroy();
    }

    if (m_VBO) {
        grr::gVertexArray::DeleteBuffer(m_VBO);
    }

    if (m_EBO) {
        grr::gVertexArray::DeleteBuffer(m_EBO);
    }

    if (m_PBO) {
        grr::gVertexArray::DeleteBuffer(m_PBO);
    }
}

void Batching::insert(Transform *transform) {
    m_models.emplace_back(transform);
}

void Batching::remove(Transform *transform) {
    m_models.erase(std::remove(m_models.begin(), m_models.end(), transform), m_models.end());

    if (m_models.empty()) {
        auto it = m_batched.find(m_name);
        if (it == m_batched.end()) {
            return;
        }
        m_batched.erase(it);
    }
}

void Batching::draw() {
    if (!m_vertexArray || !m_vertexArray->isValid()) {
        return;
    }
    setupMaterial();

    m_vertexArray->bind();

    if (bHasIndice && m_primCount) {
        grr::gVertexArray::DrawElementsInstanced(grr::PrimitiveType_Triangles, m_indiceCount, nullptr, m_primCount);
    } else if (bHasIndice) {
        return grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_indiceCount, nullptr);
    } else if (m_primCount) {
        return grr::gVertexArray::DrawArraysInstanced(grr::PrimitiveType_Triangles, m_indiceCount, m_primCount);
    }
    return grr::gVertexArray::DrawArrays(grr::PrimitiveType_Triangles, m_indiceCount);
}

void Batching::rebuild() {
    compile();
}

void Batching::compileMesh(Mesh *mesh) {
    bHasIndice = mesh->hasIndices();

    if (!m_vertexArray) {
        m_vertexArray = grr::gVertexArray::Create();
    }
    
    if (!m_VBO) {
        m_VBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    }

    if (!m_PBO) {
        m_PBO = grr::gVertexArray::CreateBuffer(grr::BufferType_PBO);
    }
    
    if (!m_EBO) {
        m_EBO = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO);
    }

    auto vertexCount = mesh->getVertices().size();
    auto vertexData  = mesh->getVertices().data();

    m_indiceCount = vertexCount;

    m_vertexArray->bind();

    if (bHasIndice) {
        auto indiceCount = mesh->getIndices().size();
        auto indiceData  = mesh->getIndices().data();

        m_indiceCount = indiceCount;

        grr::gVertexArray::Bind(m_EBO);
        grr::gVertexArray::UpdateResizeBuffer(indiceCount * sizeof(grm::u32), grr::gBufferFlags_Static);
        grr::gVertexArray::SetBufferUpdate(0, indiceCount * sizeof(grm::u32), indiceData);
    }

    grr::gVertexArray::Bind(m_VBO);
    grr::gVertexArray::UpdateResizeBuffer(vertexCount * sizeof(Vertex3D), grr::gBufferFlags_Static);
    grr::gVertexArray::SetBufferUpdate(0, vertexCount * sizeof(Vertex3D), vertexData);

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));
    grr::gVertexArray::SetAttrib(3, 1, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, mIdx)));

    grr::gVertexArray::Bind(m_PBO);
    for (int i=0;i<4;i++) {
        grr::gVertexArray::SetAttrib(4 + i, 4, sizeof(Matrix4x4), reinterpret_cast<const void*>(i * sizeof(Vector4)));
        grr::gVertexArray::SetAttribDivisor(4 + i, 1);
    }

    m_vertexArray->unbind();
}

void Batching::compile() {
    if (!m_vertexArray || !m_vertexArray->isValid() || m_models.empty()) {
        return;
    }
    std::vector<Matrix4x4> models;

    for (auto transform : m_models) {
        models.push_back(transform->getGlobalMatrix());
    }

    m_vertexArray->bind();
    grr::gVertexArray::Bind(m_PBO);
    grr::gVertexArray::UpdateResizeBuffer(models.size() * sizeof(Matrix4x4), grr::gBufferFlags_Dynamic);
    grr::gVertexArray::SetBufferUpdate(0, models.size() * sizeof(Matrix4x4), models.data());

    m_primCount = models.size();
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

void Batching::Renderer() {
    Statistics::m_drawCallCount = 0;

    
    for (auto &&[name, batch] : m_batched) {
        batch->compile();

        batch->draw();

        Statistics::m_drawCallCount++;
    }
}

Batching *Batching::Find(const std::string &name) {
    auto it = m_batched.find(name);
    if (it == m_batched.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void Batching::Delete(const std::string &name) {
    auto it = m_batched.find(name);
    if (it == m_batched.end()) {
        return;
    }
    m_batched.erase(it);
}
