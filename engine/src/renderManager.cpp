#include "renderManager.h"
#include "gameObject.h"
#include "renderer.h"

#include "component/cameraComponent.h"
#include "component/MeshRender.h"
#include "component/Transform.h"

#include "resources/resources.h"
#include "resources/shader.h"

#include "geometry/batchedGeometry.h"

#include "objectManager.h"

#include <gr-render/gVertexArray.h>

std::unique_ptr<Octree> RenderManager::m_octree = nullptr;

Shader* RenderManager::m_shaderModel = nullptr;

u32 RenderManager::drawBatchinCount = 0;
u32 RenderManager::drawCallRender = 0;
u32 RenderManager::drawCallCount = 0;
u32 RenderManager::drawCount = 0;

Frustum RenderManager::m_frustum;

grr::gVertexArray* m_vertexArray = nullptr;
u32 m_indexEBO = 0;
u32 m_indexVBO = 0;

void RenderManager::Initialize() {
    m_shaderModel = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader).string() + "/model/default3D", [](Shader*shader){
        shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
        shader->RegisterUniform("view", 1, Shader::eMAT4x4);

        shader->RegisterUniform("m_texture", 1, Shader::eINT);
    });

    m_vertexArray = grr::gVertexArray::Create();
    m_vertexArray->bind();

    m_indexEBO = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO, 1024, sizeof(u32));
    grr::gVertexArray::Bind(m_indexEBO);
    grr::gVertexArray::SetBufferUpdate(m_indexEBO, 0, 1024 * sizeof(u32), nullptr);

    m_indexVBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO, 1024, sizeof(Vertex3D));
    grr::gVertexArray::Bind(m_indexVBO);
    grr::gVertexArray::SetBufferUpdate(m_indexVBO, 0, 1024 * sizeof(Vertex3D), nullptr);

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));

    m_vertexArray->unbind();
}

bool is = false;

void RenderManager::Process() {
    if (!m_shaderModel->isvalid()) return;

    drawBatchinCount = 0;
    drawCallCount = 0;
    drawCount = 0;

    auto& batch = BatchedGeometry::m_batchins;

    if (!is) {
        m_octree = std::make_unique<Octree>(Vector3::zero, Vector3(1000, 1000, 1000));


        for (auto& object : ObjectManager::getobjects()) {
            if (!object->hasComponent<MeshRender>())
                continue;
            // auto& batchedGeometry = b.second;

            // auto& mesh = batchedGeometry->m_mesh;

            // for (auto& object : batchedGeometry->getObjects()) {
                auto& meshRender = object->getComponent<MeshRender>();
                auto& transform = object->getComponent<Transform>();

                // auto& boundBox = meshRender.m_boundBox;

                Matrix4x4 model = transform.getglobalmatrix();

                Vector3 t, s;
                Quaternion r;

                model.decompose(s, r, t);

                m_octree->insert(t, object.get());
            // }
        }
        is = true;
    }
}



void RenderManager::Renderer() {
    if (!m_octree) return;

    CameraComponent& cameraComponent = CameraComponent::MainCamera();

    m_frustum.CalculateFrustum(cameraComponent.getviewmatrix() * cameraComponent.getperspesctive());

    std::vector<GameObject*> models = m_octree->render(m_frustum);

    std::vector<Vertex3D> vertices;
    std::vector<u32> indices;
    u32 verticeCount;

    for (const auto& object : models) {
        MeshRender& meshRender = object->getComponent<MeshRender>();
        Transform& transform = object->getComponent<Transform>();

        const auto& mesh = meshRender.m_outmesh;
        const auto model = transform.getglobalmatrix();

        for (const auto& vertex : mesh.getvertices()) {
            vertices.emplace_back(Vector3(model * Vector4(vertex.position, 1.0f)));
        }
        for (const auto& index : mesh.getindices()) {
            indices.push_back(verticeCount + index);
        }
        verticeCount += mesh.getverticecount();
        // DebugDraw::addPoint(transform.position, Color::blue);
    }

    std::cout << models.size() << std::endl;

    // CameraComponent& cameraComponent = CameraComponent::MainCamera();

    m_shaderModel->bind();
    m_shaderModel->setUniform("projection", cameraComponent.getperspesctive());
    m_shaderModel->setUniform("view", cameraComponent.getviewmatrix());

    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_indexEBO);
    grr::gVertexArray::UpdateResize(m_indexEBO, indices.size(), sizeof(u32));
    grr::gVertexArray::SetBufferUpdate(m_indexEBO, 0, indices.size() * sizeof(u32), indices.data());

    grr::gVertexArray::Bind(m_indexVBO);
    grr::gVertexArray::UpdateResize(m_indexVBO, vertices.size(), sizeof(Vertex3D));
    grr::gVertexArray::SetBufferUpdate(m_indexVBO, 0, vertices.size() * sizeof(Vertex3D), vertices.data());

    grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, indices.size(), nullptr);
    /*
    if (!m_shaderModel->isvalid()) return;

    drawBatchinCount = 0;
    drawCallCount = 0;
    drawCount = 0;

    auto& batchedGeometry = BatchedGeometry::m_batchins;

    CameraComponent& cameraComponent = CameraComponent::MainCamera();

    m_shaderModel->bind();
    m_shaderModel->setUniform("projection", cameraComponent.getperspesctive());
    m_shaderModel->setUniform("view", cameraComponent.getviewmatrix());

    m_frustum.CalculateFrustum(cameraComponent.getviewmatrix() * cameraComponent.getperspesctive());

    for (auto& batch : batchedGeometry) {
        auto& batchedGeometry = batch.second;

        batchedGeometry->render(m_frustum, m_shaderModel);
    }
    */
}

void RenderManager::SetupMaterial(Shader *shader, std::unordered_map<u32, Material> &materials) {
    if (!materials[0].texture[TexturePBR::ALBEDO]->isvalid()) return;
    
    materials[0].texture[TexturePBR::ALBEDO]->bind();
    shader->setUniform("m_texture", 0);
}
