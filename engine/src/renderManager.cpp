#include "renderManager.h"

#include "objectManager.h"
#include "gameObject.h"

#include "component/cameraComponent.h"
#include "component/lightComponent.h"
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
        shader->RegisterUniform("m_lightcount", 1, Shader::eINT);
        shader->RegisterUniform("m_eye", 1, Shader::eVECTOR3);

        for (u8 i=0;i<10;i++) {
            shader->RegisterUniform("m_material[" + std::to_string(i) + "].albedo", 1, Shader::eCOLOR);
            shader->RegisterUniform("m_material[" + std::to_string(i) + "].params", 1, Shader::eVECTOR3);
        }

        for (u8 i=0;i<10;i++) {
            shader->RegisterUniform("m_light[" + std::to_string(i) + "].position",    1, Shader::eVECTOR3);
            shader->RegisterUniform("m_light[" + std::to_string(i) + "].color",       1, Shader::eCOLOR);
            shader->RegisterUniform("m_light[" + std::to_string(i) + "].attenuation", 1, Shader::eVECTOR3);
            shader->RegisterUniform("m_light[" + std::to_string(i) + "].direction",   1, Shader::eVECTOR3);
            shader->RegisterUniform("m_light[" + std::to_string(i) + "].params",      1, Shader::eVECTOR3);
            shader->RegisterUniform("m_light[" + std::to_string(i) + "].vp",          1, Shader::eMAT4x4);
        }
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
    m_shaderModel->setUniform("m_eye", camera.getposition());

    SetupLight(m_shaderModel);

    for (const auto& object : m_objects) {
        auto& transform = object->getComponent<Transform>();
        auto& meshRender = object->getComponent<MeshRender>();

        m_shaderModel->setUniform("model", transform.getglobalmatrix());

        SetupMaterial(m_shaderModel, meshRender.m_materials);

        meshRender.draw();
    }
}

void RenderManager::SetupMaterial(Shader *shader, std::unordered_map<u32, Material> &materials) {
    for (const auto& entry : materials) {
        const auto& index = entry.first;
        const auto& material = entry.second;

        shader->setUniform("m_material[" + std::to_string(index) + "].albedo", material.albedo);
        shader->setUniform("m_material[" + std::to_string(index) + "].params", Vector3(material.roughness, material.metallic, material.ao));
    }

    // if (!materials[0].texture[TexturePBR::ALBEDO]->isvalid()) return;
    
    // materials[0].texture[TexturePBR::ALBEDO]->bind();
    // shader->setUniform("m_texture", 0);
}

void RenderManager::SetupLight(Shader *shader) {
    auto& lights = LightComponent::m_lights;

    if (lights.empty()) return;

    shader->setUniform("m_lightcount", (int)lights.size());

    for (u32 i=0;i<lights.size();i++) {
        const auto& light = lights[i];

        shader->setUniform("m_light[" + std::to_string(i) + "].position",    light->getposition());
        shader->setUniform("m_light[" + std::to_string(i) + "].color",       light->getcolor());
        shader->setUniform("m_light[" + std::to_string(i) + "].attenuation", Vector3(light->getattenuationconstant(), light->getattenuationlinear(), light->getattenuationquadractc()));
        shader->setUniform("m_light[" + std::to_string(i) + "].direction",   light->getforward());
        shader->setUniform("m_light[" + std::to_string(i) + "].params",      Vector3(light->getinnecone(), light->getoutercone(), (float)light->gettype()));
        shader->setUniform("m_light[" + std::to_string(i) + "].vp",            light->getprojectionview());
    }
}
