#include "batching.hpp"

#include "renderManager.h"
#include "mesh.hpp"

#include "core/assetsManager.hpp"
#include "core/statistics.hpp"
#include "core/event.hpp"

#include "script/transform.hpp"
#include "script/light.hpp"

#include "culling/frustum.h"

#include "scriptManager/scriptManager.hpp"

#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

std::unordered_map<std::string, std::unique_ptr<Batching>> Batching::s_batchedInstances;

std::vector<Batching *> Batching::s_batchedInstancesList;

grr::gShader *Batching::s_shadowShader = nullptr;

void Batching::Initialize() {
    AssetsManager::LoadShader("shadow", File::GetAsset("/shader/shadow/shadow.frag"), File::GetAsset("/shader/shadow/shadow.vert"), &Batching::RegisterShaderShadow);
}

// TODO: falta descarregar o shader
void Batching::Shutdown() {
    if (s_shadowShader) {
        s_shadowShader = nullptr;
    }
}

void Batching::RenderShadowPass() {
    if (!s_shadowShader || !ScriptManager::GetListLight) {
        return;
    }

    auto& lights = ScriptManager::GetListLight();
    if (lights.empty()) { return; }

    grr::gShader::Bind(s_shadowShader);

    grr::gShader::SetUniform("u_light", lights[0]->getProjetion());

    for (auto &batch : s_batchedInstancesList) {
        if (!batch || !batch->hasModel()) {
            continue;
        }
        
        batch->draw(s_shadowShader, false);
    }

    grr::gShader::Unbind();
}

grm::size Batching::GetBatchCount() {
    return s_batchedInstancesList.size();
}

Batching *Batching::FindBatchByName(const std::string &name) {
    auto it = s_batchedInstances.find(name);
    if (it == s_batchedInstances.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void Batching::RemoveBatchByName(const std::string &name) {
    auto it = s_batchedInstances.find(name);
    if (it == s_batchedInstances.end()) {
        return;
    }

    s_batchedInstancesList.erase(std::remove(s_batchedInstancesList.begin(), s_batchedInstancesList.end(), &(*it->second)), s_batchedInstancesList.end());

    s_batchedInstances.erase(it);
}

void Batching::RenderBatches() {
    Statistics::m_drawRenderCount = 0;
    Statistics::m_drawCallCount = 0;
    Statistics::m_totalRender = 0;

    for (auto &batching : s_batchedInstancesList) {
        if (!batching || !batching->hasModel()) {
            continue;
        }
        
        batching->draw();
    
        Statistics::m_drawCallCount++;
    }
}

Batching::Batching(const std::string &name, std::vector<std::shared_ptr<Material>> *materials, grr::gShader *shader, BatchingFlags_ flags) : m_shader(shader), m_flags(flags), m_name(name), m_materials(materials), m_vertexArray(nullptr), m_indexCount(0), m_instanceCount(0), m_VBO(0), m_EBO(0), m_PBO(0), m_type(DrawType_Elements) {}

Batching::~Batching() {
    if (m_vertexArray) {
        grr::gVertexArray::Destroy(m_vertexArray);
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

void Batching::buildMeshe(Mesh *mesh) {
    m_vertexArray = m_vertexArray != nullptr ? m_vertexArray : grr::gVertexArray::Create();

    m_VBO = m_VBO != 0 ? m_VBO : grr::gVertexArray::CreateBuffer(BufferType::VBO);
    
    auto vertices = mesh->getVertices();

    m_indexCount = vertices.size();
    
    m_vertexArray->bind();

    if (m_flags & BatchingFlags_HasIndices) {
        m_EBO = m_EBO != 0 ? m_EBO : grr::gVertexArray::CreateBuffer(BufferType::EBO);

        auto indices = mesh->getIndices();

        m_indexCount = indices.size();

        grr::gVertexArray::Bind(m_EBO);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::u32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::u32), indices.data());
    }

    grr::gVertexArray::Bind(m_VBO);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex3D), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex3D), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));
    grr::gVertexArray::SetAttrib(3, 1, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, mIdx)));

    m_vertexArray->unbind();

    m_flags |= BatchingFlags_BuildMesh;
}

void Batching::insert(Transform *transform) {
    m_models.emplace_back(transform);
}

void Batching::remove(Transform *transform, bool bDelete) {
    m_models.erase(std::remove(m_models.begin(), m_models.end(), transform), m_models.end());

    if (m_models.empty() && bDelete) {
        Batching::RemoveBatchByName(m_name);
    }
}

void Batching::draw(grr::gShader *shader, bool bSetMaterial) {
    shader = shader != nullptr ? shader : m_shader;
    
    grr::gShader::Bind(shader);

    grr::gShader::SetUniform("u_model", m_models[0]->getGlobalMatrix());
    
    if (bSetMaterial)
        setupMaterial();

    m_vertexArray->bind();

    grr::gShader::Flush();

    /* switch (m_type) {
    case DrawType_Elements_Instanced:
        grr::gVertexArray::DrawElementsInstanced(grr::PrimitiveType_Triangles, m_indexCount, nullptr, m_instanceCount);
        break;
    case DrawType_Arrays_Instanced:
        grr::gVertexArray::DrawArraysInstanced(grr::PrimitiveType_Triangles, m_indexCount, m_instanceCount);
        break;
    case DrawType_Elements:
        break;
    case DrawType_Arrays: */
    if (m_flags & BatchingFlags_HasIndices) {
        grr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_indexCount, nullptr);
    } else {
        grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, m_indexCount);
    }
        /*break;
    default:
        break;
    }
    */
}

/*
void Batching::BuildMeshe(Mesh *mesh) {
    m_hasIndex = mesh->hasIndices();

    if (!m_vertexArray) {
        m_vertexArray = grr::gVertexArray::Create();
    }
    
    if (!m_VBO) {
        m_VBO = grr::gVertexArray::CreateBuffer(BufferType::VBO);
    }

    if (!m_PBO && m_flags == BatchingType_Instanced_Dynamic || m_flags == BatchingType_Instanced_Static) {
        m_PBO = grr::gVertexArray::CreateBuffer(grr::BufferType_PBO);
    }
    
    if (!m_EBO && m_hasIndex) {
        m_EBO = grr::gVertexArray::CreateBuffer(BufferType::EBO);
    }

    auto vertexCount = mesh->getVertices().size();
    auto vertexData  = mesh->getVertices().data();

    m_indexCount = vertexCount;

    m_vertexArray->bind();

    if (m_hasIndex) {
        auto indiceCount = mesh->getIndices().size();
        auto indiceData  = mesh->getIndices().data();

        m_indexCount = indiceCount;

        grr::gVertexArray::Bind(m_EBO);
        grr::gVertexArray::UpdateResizeBuffer(indiceCount * sizeof(grm::u32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indiceCount * sizeof(grm::u32), indiceData);
    }

    grr::gVertexArray::Bind(m_VBO);
    grr::gVertexArray::UpdateResizeBuffer(vertexCount * sizeof(Vertex3D), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertexCount * sizeof(Vertex3D), vertexData);

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));
    grr::gVertexArray::SetAttrib(3, 1, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, mIdx)));

    if (m_flags == BatchingType_Instanced_Dynamic || m_flags == BatchingType_Instanced_Static) {
        grr::gVertexArray::Bind(m_PBO);
        for (int i=0;i<4;i++) {
            grr::gVertexArray::SetAttrib(4 + i, 4, sizeof(Matrix4x4), reinterpret_cast<const void*>(i * sizeof(Vector4)));
            grr::gVertexArray::SetAttribDivisor(4 + i, 1);
        }
    }

    m_vertexArray->unbind();

    // process boundbox
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    
    for (const auto& vertex : mesh->getVertices()) {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
}

void Batching::CompileBatch() {
    switch (m_flags) {
    case BatchingType_Static_Mesh:
        buildStaticMesh();
        break;
    case BatchingType_Instanced_Static:
        buildStaticInstanced();
        break;
    case BatchingType_Instanced_Dynamic:
        buildDynamicInstanced();
        break;
    case BatchingType_Dynamic:
        buildDynamic();
        break;
    default:
        break;
    }

    Statistics::m_totalRender += m_models.size();
}

void Batching::buildDynamic() {
    if (m_models.empty()) {
        return;
    }

    grr::gShader::SetUniform("u_model", m_models[0]->getGlobalMatrix());
}
*/
/*
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

    m_instanceCount = models.size();
}

void Batching::buildStaticInstanced() {
    if (m_instanceCount == m_models.size()) {
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
    grr::gVertexArray::UpdateResizeBuffer(models.size() * sizeof(Matrix4x4), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, models.size() * sizeof(Matrix4x4), models.data());

    m_type = DrawType_Arrays_Instanced;

    m_instanceCount = models.size();
}

void Batching::buildStaticMesh() {
    if (m_instanceCount == m_models.size()) {
        return;
    }
    Mesh mesh;

    grm::size verticeCount = 0;
    for (auto &transform : m_models) {
        auto model = transform->getGlobalMatrix();

        for (auto &vertice : m_mesh->getVertices()) {
            Vector4 position = {vertice.position.x, vertice.position.y, vertice.position.z, 1.0f};
            Vector4 normal = {vertice.normal.x, vertice.normal.y, vertice.normal.z, 0.0f};

            mesh.addVertex(
                Vector3(model * position),
                Vector3(model * normal),
                vertice.texCoord,
                vertice.mIdx
            );
        }

        for (auto &indice : m_mesh->getIndices()) {
            mesh.addIndice(verticeCount + indice);
        }
        verticeCount += m_mesh->getVertices().size();
    }
    
    BuildMeshe(&mesh);

    m_type = m_hasIndex ? DrawType_Elements : DrawType_Arrays;

    m_instanceCount = m_models.size();
}
*/

void Batching::setupMaterial() {
    int idx[10] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idxEn[10] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (auto i=0;i<m_materials->size();i++) {
        auto &material = (*m_materials)[i];

        for (auto i=0;i<10;++i) {
            auto texture = material->getTexture(i);
            if (!texture || !texture->isValid()) {
                continue;
            }
            grr::gTexture::Bind(texture, idx[i]);
            idxEn[i] = 1;
        }

        grr::gShader::SetUniform("m_texture", (const int *)idx);
        grr::gShader::SetUniform("m_activate", (const int *)idxEn);

        grr::gShader::SetUniform("m_material[" + std::to_string(i) + "].albedo", material->getAlbedo());
        grr::gShader::SetUniform("m_material[" + std::to_string(i) + "].params", Vector3({material->getRoughness(), material->getMetallic(), material->getAmbientOcclusion()}));
    }
}

void Batching::RegisterShaderShadow(grr::gShader *shader) {
    if (!shader) {
        return;
    }

    grr::gShader::Bind(shader);

    grr::gShader::Register("u_model", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    grr::gShader::Register("u_light", UniformType::MAT4, 1, Matrix4x4::identityMatrix);

    grr::gShader::Unbind();
    
    s_shadowShader = shader;
}

bool Batching::hasModel() const {
    return !m_models.empty();
}







