#include "renderManager.h"
#include "gameObject.h"

#include "component/cameraComponent.h"
#include "component/MeshRender.h"
#include "component/Transform.h"

#include "geometry/batchedGeometry.h"

#include "resources/resources.h"
#include "resources/shader.h"

#include "culling/frustum.h"

Shader* RenderManager::m_shaderModel = nullptr;

u32 RenderManager::drawBatchinCount = 0;
u32 RenderManager::drawCallRender = 0;
u32 RenderManager::drawCallCount = 0;
u32 RenderManager::drawCount = 0;

void RenderManager::Initialize() {
    m_shaderModel = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader).string() + "/model/default3D", [](Shader*shader){
        shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
        shader->RegisterUniform("view", 1, Shader::eMAT4x4);

        shader->RegisterUniform("m_texture", 1, Shader::eINT);
    });
}

void RenderManager::Process() {
}

void RenderManager::Renderer() {
    if (!m_shaderModel->isvalid()) return;

    drawBatchinCount = 0;
    drawCallCount = 0;
    drawCount = 0;

    auto& batch = BatchedGeometry::m_batchins;

    CameraComponent& cameraComponent = CameraComponent::MainCamera();

    m_shaderModel->bind();
    m_shaderModel->setUniform("projection", cameraComponent.getperspesctive());
    m_shaderModel->setUniform("view", cameraComponent.getviewmatrix());

    Frustum m_frustum;
    m_frustum.CalculateFrustum(cameraComponent.getviewmatrix() * cameraComponent.getperspesctive());

    for (auto& b : batch) {
        auto& batchedGeometry = b.second;

        auto bufferManager = batchedGeometry->getBufferManager();
        auto& mesh = batchedGeometry->m_mesh;

        std::vector<Vertex3D> vertices;
        std::vector<u32> indices;
        u32 vertexCount = 0;

        for (const auto& object : batchedGeometry->getObjects()) {
            auto& meshRender = object->getComponent<MeshRender>();
            auto& transform = object->getComponent<Transform>();

            Matrix4x4 model = transform.getglobalmatrix();

            auto& boundBox = meshRender.m_boundBox;

            if (!m_frustum.IsCubeInFrustumOOBB(boundBox.min, boundBox.max, model)) {
                continue;
            }

            for (const auto& vertex : mesh.getvertices()) {
                Vector4 position = model * Vector4(vertex.position, 1.0f);
                vertices.emplace_back(Vector3(position), vertex.normal, vertex.uv, vertex.tangent, vertex.bitangent, vertex.materialIndex);
            }

            for (const auto& index : mesh.getindices()) {
                indices.push_back(vertexCount + index);
            }

            vertexCount += mesh.getvertices().size();

            drawBatchinCount++;
        }

        if (vertices.empty() || indices.empty()) {
            continue;
        }

        drawCount += batchedGeometry->getObjects().size();

        SetupMaterial(m_shaderModel, batchedGeometry->m_materials);
        
        bufferManager->bind(BufferManager::eVAO);

        bufferManager->bind(BufferManager::eEBO);
        bufferManager->pushUpdateElements(0, indices.size() * sizeof(u32), indices.data());

        bufferManager->bind(BufferManager::eVBO);
        bufferManager->pushUpdateBuffer(0, vertices.size() * sizeof(Vertex3D), vertices.data());

        bufferManager->draw(BufferManager::eTriangle, indices.size(), BufferManager::eUint, nullptr);

        drawCallCount++;
    }
}

void RenderManager::SetupMaterial(Shader *shader, std::unordered_map<u32, Material> &materials) {
    if (!materials[0].texture[TexturePBR::ALBEDO]->isvalid()) return;
    
    materials[0].texture[TexturePBR::ALBEDO]->bind();
    shader->setUniform("m_texture", 0);
}
