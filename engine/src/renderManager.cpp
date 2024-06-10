#include "renderManager.h"

#include "objectManager.h"
#include "gameObject.h"

#include "component/cameraComponent.h"
#include "component/MeshRender.h"
#include "component/Transform.h"

#include "resources/resources.h"
#include "resources/shader.h"

#include "geometry/batchedGeometry.h"

std::unique_ptr<Octree> RenderManager::m_octree = nullptr;

std::vector<GameObject*> RenderManager::m_objects;

Shader* RenderManager::m_shaderModel = nullptr;

u32 RenderManager::drawBatchinCount = 0;
u32 RenderManager::drawCallRender = 0;
u32 RenderManager::drawCallCount = 0;
u32 RenderManager::drawCount = 0;

Frustum RenderManager::m_frustum;


void RenderManager::Initialize() {
    m_shaderModel = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader).string() + "/model/default3D", [](Shader*shader){
        shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
        shader->RegisterUniform("view", 1, Shader::eMAT4x4);
        shader->RegisterUniform("model", 1, Shader::eMAT4x4);

        shader->RegisterUniform("m_texture", 1, Shader::eINT);
    });
}

void RenderManager::Process() {
    m_objects.clear();

    auto& camera = CameraComponent::MainCamera();

    m_frustum.CalculateFrustum(camera.getperspesctive() * camera.getviewmatrix());

    for (auto& object : ObjectManager::getobjects()) {
        if (!object->hasComponent<MeshRender>()) {
            continue;
        }

        auto& meshRender = object->getComponent<MeshRender>();
        auto& transform = object->getComponent<Transform>();

        auto& boundBox = meshRender.getBoundBox();
        auto& model = transform.getglobalmatrix();

        // if (!m_frustum.IsCubeInFrustumOOBB(boundBox.min, boundBox.max, model)) {
        //     continue;
        // }
        m_objects.push_back(object.get());
    }
}



void RenderManager::Renderer() {
    if (!m_shaderModel->isvalid() || m_objects.empty()) return;

    auto& camera = CameraComponent::MainCamera();

    m_shaderModel->bind();
    m_shaderModel->setUniform("projection", camera.getperspesctive());
    m_shaderModel->setUniform("view", camera.getviewmatrix());

    for (const auto& object : m_objects) {
        auto& transform = object->getComponent<Transform>();
        auto& meshRender = object->getComponent<MeshRender>();

        m_shaderModel->setUniform("model", transform.getglobalmatrix());

        meshRender.draw();
    }
}

void RenderManager::SetupMaterial(Shader *shader, std::unordered_map<u32, Material> &materials) {
    if (!materials[0].texture[TexturePBR::ALBEDO]->isvalid()) return;
    
    materials[0].texture[TexturePBR::ALBEDO]->bind();
    shader->setUniform("m_texture", 0);
}
