#include "batching.hpp"

#include "renderManager.h"
#include "mesh.h"

#include "core/statistics.hpp"
#include "core/event.hpp"

#include "script/transform.hpp"

#include "culling/frustum.h"

#include <cstring>

#include <gr-render/gVertexArray.h>
#include <gr-render/gShader.h>

std::unordered_map<std::string, std::unique_ptr<Batching>> Batching::m_batchedMap;

std::vector<Batching *> Batching::m_batched;

grm::size Batching::GetBatchedCount() {
    return m_batched.size();
}

Batching *Batching::Find(const std::string &name) {
    auto it = m_batchedMap.find(name);
    if (it == m_batchedMap.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void Batching::Delete(const std::string &name) {
    auto it = m_batchedMap.find(name);
    if (it == m_batchedMap.end()) {
        return;
    }

    m_batched.erase(std::remove(m_batched.begin(), m_batched.end(), &(*it->second)), m_batched.end());

    m_batchedMap.erase(it);
}

void Batching::Renderer() {
    Statistics::m_drawRenderCount = 0;
    Statistics::m_drawCallCount = 0;
    Statistics::m_totalRender = 0;

    std::for_each(m_batched.begin(), m_batched.end(), &Batching::Draw);
}

Batching::Batching(const std::string &name, std::vector<Material> *materials, Mesh *mesh, grr::gShader *shader, int flags) : m_shader(shader), m_flags(flags), m_mesh(mesh), m_name(name), m_materials(materials), m_vertexArray(nullptr), m_indiceCount(0), m_primCount(0), m_VBO(0), m_EBO(0), m_PBO(0), bHasIndice(false), m_type(DrawType_Elements) {
    if (flags != Batching_Flags_Static_Mesh)
        build(mesh);
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

void Batching::remove(Transform *transform, bool bDelete) {
    m_models.erase(std::remove(m_models.begin(), m_models.end(), transform), m_models.end());

    if (m_models.empty() && bDelete) {
        Batching::Delete(m_name);
    }
}

void Batching::draw() {
    grr::gShader::Bind(m_shader);

    compile();
    
    if (!m_vertexArray || !m_shader || !m_vertexArray->isValid() || !m_shader->isValid()) {
        return;
    }

    setupMaterial();

    m_vertexArray->bind();

    switch (m_type) {
    case DrawType_Elements_Instanced:
        grr::gVertexArray::DrawElementsInstanced(grr::PrimitiveType_Triangles, m_indiceCount, nullptr, m_primCount);
        break;
    case DrawType_Arrays_Instanced:
        grr::gVertexArray::DrawArraysInstanced(grr::PrimitiveType_Triangles, m_indiceCount, m_primCount);
        break;
    case DrawType_Elements:
        grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_indiceCount, nullptr);
        break;
    case DrawType_Arrays:
        grr::gVertexArray::DrawArrays(grr::PrimitiveType_Triangles, m_indiceCount);
        break;
    default:
        throw std::runtime_error("Draw type invalid.");
        break;
    }
}

void Batching::Draw(Batching *batching) {
    if (!batching->hasModel()) {
        return;
    }
    
    batching->draw();

    Statistics::m_drawCallCount++;
}

void Batching::build(Mesh *mesh) {
    bHasIndice = mesh->hasIndices();

    if (!m_vertexArray) {
        m_vertexArray = grr::gVertexArray::Create();
    }
    
    if (!m_VBO) {
        m_VBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    }

    if (!m_PBO && m_flags == Batching_Flags_Instanced_Dynamic || m_flags == Batching_Flags_Instanced_Static) {
        m_PBO = grr::gVertexArray::CreateBuffer(grr::BufferType_PBO);
    }
    
    if (!m_EBO && bHasIndice) {
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

    if (m_flags == Batching_Flags_Instanced_Dynamic || m_flags == Batching_Flags_Instanced_Static) {
        grr::gVertexArray::Bind(m_PBO);
        for (int i=0;i<4;i++) {
            grr::gVertexArray::SetAttrib(4 + i, 4, sizeof(Matrix4x4), reinterpret_cast<const void*>(i * sizeof(Vector4)));
            grr::gVertexArray::SetAttribDivisor(4 + i, 1);
        }
    }

    m_vertexArray->unbind();

    // process boundbox
    m_boundbox.min = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    m_boundbox.max = Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    
    for (const auto& vertex : mesh->getVertices()) {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
}

void Batching::compile() {
    switch (m_flags) {
    case Batching_Flags_Static_Mesh:
        buildStaticMesh();
        break;
    case Batching_Flags_Instanced_Static:
        buildStaticInstanced();
        break;
    case Batching_Flags_Instanced_Dynamic:
        buildDynamicInstanced();
        break;
    case Batching_Flags_Dynamic:
        buildDynamic();
        break;
    default:
        throw std::runtime_error(".");
        break;
    }

    Statistics::m_totalRender += m_models.size();
}

void Batching::buildDynamic() {
    grr::gShader::SetUniform("u_model", 1, m_models[0]->getGlobalMatrix());

    m_type = DrawType_Arrays;
}

void Batching::buildDynamicInstanced() {
    std::vector<Matrix4x4> models;
    for (auto &transform : m_models) {
        models.push_back(transform->getGlobalMatrix());
    }

    if (models.empty()) {
        return;
    }

    m_vertexArray->bind();
    grr::gVertexArray::Bind(m_PBO);
    grr::gVertexArray::UpdateResizeBuffer(models.size() * sizeof(Matrix4x4), grr::gBufferFlags_Dynamic);
    grr::gVertexArray::SetBufferUpdate(0, models.size() * sizeof(Matrix4x4), models.data());

    m_type = DrawType_Arrays_Instanced;

    m_primCount = models.size();
}

void Batching::buildStaticInstanced() {
    if (m_primCount == m_models.size()) {
        return;
    }

    std::vector<Matrix4x4> models;
    for (auto &transform : m_models) {
        models.push_back(transform->getGlobalMatrix());
    }

    if (models.empty()) {
        return;
    }

    m_vertexArray->bind();
    grr::gVertexArray::Bind(m_PBO);
    grr::gVertexArray::UpdateResizeBuffer(models.size() * sizeof(Matrix4x4), grr::gBufferFlags_Static);
    grr::gVertexArray::SetBufferUpdate(0, models.size() * sizeof(Matrix4x4), models.data());

    m_type = DrawType_Arrays_Instanced;

    m_primCount = models.size();
}

void Batching::buildStaticMesh() {
    if (m_primCount == m_models.size()) {
        return;
    }
    Mesh mesh;

    grm::size verticeCount = 0;
    for (auto &transform : m_models) {
        auto &model = transform->getGlobalMatrix();

        for (auto &vertice : m_mesh->getVertices()) {
            mesh.addVertex(
                Vector3(model * Vector4(vertice.position, 1.0f)),
                Vector3(model * Vector4(vertice.normal, 0.0f)),
                vertice.texCoord,
                vertice.mIdx
            );
        }

        for (auto &indice : m_mesh->getIndices()) {
            mesh.addIndice(verticeCount + indice);
        }
        verticeCount += m_mesh->getVertices().size();
    }
    
    build(&mesh);

    m_type = DrawType_Arrays;

    m_primCount = m_models.size();
}

void Batching::setupMaterial() {
    int idx[10] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idxEn[10] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    auto &materials = *m_materials;

    for (auto i=0;i<materials.size();i++) {
        auto &material = materials[i];
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

        grr::gShader::SetUniform("m_material[" + std::to_string(i) + "].albedo", material.getAlbedo());
        grr::gShader::SetUniform("m_material[" + std::to_string(i) + "].params", Vector3(material.getRoughness(), material.getMetallic(), material.getAmbientOcclusion()));
    }
}

bool Batching::hasModel() const {
    return !m_models.empty();
}
