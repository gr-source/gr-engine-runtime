#include "renderManager.h"
#include "gameObject.h"

#include "component/cameraComponent.h"
#include "component/MeshRender.h"
#include "component/Transform.h"

#include "geometry/batchedGeometry.h"

#include "resources/resources.h"
#include "resources/shader.h"

Shader* m_shaderModel = nullptr;

const char* fragment = R"(
    #version 330 core

    out vec4 FragColor;

    void main() {
        FragColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
)";

const char* vertex = R"(
    #version 330 core

    layout (location = 0) in vec3 in_pos;

    uniform mat4 projection;
    uniform mat4 view;

    void main() {
        gl_Position = projection * view * vec4(in_pos, 1.0);
    }
)";

void RenderManager::Initialize() {
    m_shaderModel = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader).string() + "/Terrain", [](Shader*shader){
        shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
        shader->RegisterUniform("view", 1, Shader::eMAT4x4);
    });
}

void RenderManager::Process() {
    // Adicionar código de processamento, se necessário.
}

void RenderManager::Renderer() {
    if (!m_shaderModel->isvalid()) return;

    auto& batch = BatchedGeometry::m_batchins; // Presumo que haja um método estático que retorne os batches.

    CameraComponent& cameraComponent = CameraComponent::MainCamera();

    m_shaderModel->bind();
    m_shaderModel->setUniform("projection", cameraComponent.getperspesctive());
    m_shaderModel->setUniform("view", cameraComponent.getviewmatrix());

    for (auto& b : batch) {
        auto& batchedGeometry = b.second;

        auto bufferManager = batchedGeometry->getBufferManager();
        auto& mesh = batchedGeometry->m_mesh;

        std::vector<Vertex3D> vertices;
        std::vector<u32> indices;
        u32 vertexCount = 0;

        for (const auto& object : batchedGeometry->getObjects()) {
            auto& transform = object->getComponent<Transform>();

            Matrix4x4 model = transform.getglobalmatrix();

            for (const auto& vertex : mesh.getvertices()) {
                Vector4 position = model * Vector4(vertex.position, 1.0f);
                vertices.emplace_back(Vector3(position), vertex.normal, vertex.uv, vertex.tangent, vertex.bitangent, vertex.materialIndex);
            }

            for (const auto& index : mesh.getindices()) {
                indices.push_back(vertexCount + index);
            }

            vertexCount += vertices.size();
        }
        
        bufferManager->bind(BufferManager::eVAO);

        bufferManager->bind(BufferManager::eEBO);
        bufferManager->pushUpdateElements(0, indices.size() * sizeof(u32), indices.data());

        bufferManager->bind(BufferManager::eVBO);
        bufferManager->pushUpdateBuffer(0, vertices.size() * sizeof(Vertex3D), vertices.data());

        bufferManager->draw(BufferManager::eTriangle, indices.size(), BufferManager::eUint, nullptr);
    }
}
