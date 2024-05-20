#include "batchingManager.h"

#include "objectManager.h"

#include "component/cameraComponent.h"
#include "component/lightComponent.h"
#include "component/MeshRender.h"
#include "component/Rigidbody.h"
#include "component/Transform.h"
#include "component/animator.h"

#include "resources/Resources.h"

#include "framebuffer/shadowbuffer.h"
#include "framebuffer/skyboxbuffer.h"
#include "framebuffer/gBuffer.h"

#include "Global.h"

#include "skybox.h"

#include "geometry/vertex2D.h"

u32 BatchingManager::frustomDraw = 0;
u32 BatchingManager::drawCount = 0;

Skybox* BatchingManager::m_skybox = nullptr;

Shader* BatchingManager::m_shaderLights = nullptr;

Shader* BatchingManager::m_shader = nullptr;

Frustum BatchingManager::m_frustum;

CameraComponent* BatchingManager::m_cameraComponent = nullptr;

std::vector<GameObject*> BatchingManager::m_objectrender;

std::unique_ptr<BufferManager> m_screenRender = nullptr;

Shader* m_shaderShadow = nullptr;

Shadowbuffer* BatchingManager::m_shadowbuffer = nullptr;

void BatchingManager::Initialize() {
    Occlusion::Initialize();
    
    m_skybox = &Skybox::getInstance();

    m_shadowbuffer = &Shadowbuffer::getInstance();

    m_shader = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader) + "/FilmboxShader", BatchingManager::SetupEnableShaderModel);

    m_shaderLights = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader) + "/ShaderLight", BatchingManager::SetupEnableShaderLight);

    m_shaderShadow = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader) + "/ShaderShadow", BatchingManager::SetupEnableShaderShadow);

    std::vector<Vertex2D> vertice = {
        {Vector2(-1.0f,  1.0f), Vector2(0.0f, 1.0f)},
        {Vector2(-1.0f, -1.0f), Vector2(0.0f, 0.0f)},
        {Vector2( 1.0f, -1.0f), Vector2(1.0f, 0.0f)},
        {Vector2(-1.0f,  1.0f), Vector2(0.0f, 1.0f)},
        {Vector2( 1.0f, -1.0f), Vector2(1.0f, 0.0f)},
        {Vector2( 1.0f,  1.0f), Vector2(1.0f, 1.0f)}
    };

    m_screenRender = std::make_unique<BufferManager>(BufferManager::eSTATIC);
    
    m_screenRender->bind(BufferManager::eVAO);

    m_screenRender->bind(BufferManager::eVBO);
    m_screenRender->pushBuffer(vertice.size() * sizeof(Vertex2D), vertice.data());

    m_screenRender->attribPointer(0, 2, sizeof(Vertex2D), reinterpret_cast<void*>(offsetof(Vertex2D, position)));
    m_screenRender->attribPointer(1, 2, sizeof(Vertex2D), reinterpret_cast<void*>(offsetof(Vertex2D, uv)));

    m_screenRender->unbind();
}

void BatchingManager::OnUpdate() {
    m_objectrender.clear();

    m_cameraComponent = &CameraComponent::MainCamera();

    const Matrix4x4 vp = m_cameraComponent->getperspesctive() * m_cameraComponent->getviewmatrix();

    m_frustum.CalculateFrustum(vp);

    for (auto& gameObject : ObjectManager::getobjects()) {
        if (!gameObject->hasComponent<MeshRender>()) continue;

        MeshRender& meshRender = gameObject->getComponent<MeshRender>();

        Transform& transform = gameObject->getComponent<Transform>();

        const Vector3& min = meshRender.m_drawcall.m_boundBox.min;
        const Vector3& max = meshRender.m_drawcall.m_boundBox.max;

        if (!m_frustum.IsCubeInFrustumOOBB(min, max, transform.getglobalmatrix()) /*&& !meshRender.m_occlusion->isvisible(vp, transform.getglobalmatrix())*/) {
            continue;
        }

        if (gameObject->hasComponent<Animator>()) {
            Animator& animator = gameObject->getComponent<Animator>();
            animator.OnUpdate();
        }

        m_objectrender.push_back(gameObject.get());
    }
    frustomDraw = m_objectrender.size();
}

void BatchingManager::OnRenderShadow() {
    auto& lights = LightComponent::m_lights;
    if (!m_shaderShadow->isvalid() || lights.empty()) return;

    m_shaderShadow->bind();

    for (u32 i=0;i<lights.size();i++) {
        m_shaderShadow->setUniform("vp", lights[i]->getprojectionview());
        RenderModel(m_shaderShadow);
    }

    m_shaderShadow->unbind();
}

void BatchingManager::OnRender() {
    if (!m_shader->isvalid() || !m_shaderShadow->isvalid() || m_objectrender.empty()) return;

    m_shader->bind();

    m_shader->setUniform("projection", m_cameraComponent->getperspesctive());
    m_shader->setUniform("view", m_cameraComponent->getviewmatrix());
    m_shader->setUniform("gEye", m_cameraComponent->getposition());

    SetupLight(m_shader);

    for (auto& gameObject : m_objectrender) {
        if (!gameObject->hasComponent<MeshRender>()) continue;

        m_shader->setUniform("isanimation", static_cast<int>(gameObject->hasComponent<Animator>()));

        if (gameObject->hasComponent<Animator>()) {

            Animator& animator = gameObject->getComponent<Animator>();
            m_shader->setUniform("u_boneMatrices", 100, animator.m_FinalBoneMatrices.data());
        }

        MeshRender& meshRender = gameObject->getComponent<MeshRender>();

        Transform& transform = gameObject->getComponent<Transform>();

        m_shader->setUniform("model", transform.getglobalmatrix());

        SetupMaterial(meshRender, m_shader);

        meshRender.m_drawcall.buffers.bind(BufferManager::eVAO);

        meshRender.m_drawcall.buffers.draw(BufferManager::eTriangle, meshRender.indiceCount, BufferManager::eUint, nullptr);

        meshRender.m_drawcall.buffers.unbind();
        
        for (u32 i=0;i<MAX_TEXTURE;i++) {
            GDisableTexture(i);
        }
    }
    m_shader->unbind();
}

void BatchingManager::RenderModel(Shader *shader) {
    for (const auto& gameObject : m_objectrender) {
        if (!gameObject->hasComponent<MeshRender>()) continue;

        const bool hasAnimation = gameObject->hasComponent<Animator>();

        shader->setUniform("isanimation", static_cast<int>(hasAnimation));

        if (hasAnimation) {
            Animator& animator = gameObject->getComponent<Animator>();
            shader->setUniform("u_boneMatrices", 100, animator.m_FinalBoneMatrices.data());
        }

        Transform& transform = gameObject->getComponent<Transform>();
        shader->setUniform("model", transform.getglobalmatrix());

        MeshRender& meshRender = gameObject->getComponent<MeshRender>();
        meshRender.m_drawcall.buffers.bind(BufferManager::eVAO);

        meshRender.m_drawcall.buffers.draw(BufferManager::eTriangle, meshRender.indiceCount, BufferManager::eUint, nullptr);

        meshRender.m_drawcall.buffers.unbind();
    }
}

/*
void BatchingManager::OnRenderLight(GBuffer* gbuffer) {
    if (!m_shaderLights->isvalid()) return;
    
    CameraComponent& m_camera = CameraComponent::MainCamera();

    m_shaderLights->bind();

    gbuffer->GetTexture(GBuffer::Texture_Position)->bind(GL_TEXTURE0 + 0);
    m_shaderLights->setUniform("gPosition", 0);

    gbuffer->GetTexture(GBuffer::Texture_Normal)->bind(GL_TEXTURE0 + 1);
    m_shaderLights->setUniform("gNormal", 1);

    gbuffer->GetTexture(GBuffer::Texture_Albedo)->bind(GL_TEXTURE0 + 2);
    m_shaderLights->setUniform("gAlbedo", 2);

    gbuffer->GetTexture(GBuffer::Texture_Roughnness)->bind(GL_TEXTURE0 + 3);
    m_shaderLights->setUniform("gRoughness", 3);

    gbuffer->GetTexture(GBuffer::Texture_Ao)->bind(GL_TEXTURE0 + 4);
    m_shaderLights->setUniform("gAo", 4);

    gbuffer->GetTexture(GBuffer::Texture_Metallic)->bind(GL_TEXTURE0 + 5);
    m_shaderLights->setUniform("gMetallic", 5);

    m_shadowbuffer->GetTexture()->bind(GL_TEXTURE0 + 6);
    m_shaderLights->setUniform("gShadow", 6);

    m_shaderLights->setUniform("gEye", m_camera.getposition());

    auto& lights = LightComponent::m_lights;

    m_shaderLights->setUniform("m_lightcount", static_cast<int>(lights.size()));

    // Matrix4x4 projection = Math::orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 7.5f, 1.0f);
    
    for (u32 i=0;i<lights.size();i++) {
        const auto& m_light = lights[i];

        // Matrix4x4 projection = Math::orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 7.5f, 1.0f);

        m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].position", m_light->getposition());
        m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].color", m_light->getcolor());
        m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].attenuationConstant", m_light->getattenuationconstant());
        m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].attenuationLinear", m_light->getattenuationlinear());
        m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].attenuationQuadratic", m_light->getattenuationquadractc());
        m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].type", m_light->gettype());
        m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].direction", m_light->getforward());
        m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].angleInnerCone", m_light->getinnecone());
        m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].angleOuterCone", m_light->getoutercone());
        // m_shaderLights->setUniform("m_lights[" + std::to_string(i) + "].vp", projection * m_light->getviewmatrix());
    }

    m_screenRender->bind(BufferManager::eVAO);
    m_screenRender->draw(BufferManager::eTriangle, 0, 6);
}
*/

void BatchingManager::SetupMaterial(const MeshRender &meshRender, Shader *shader) {
    for (const auto& material : meshRender.materials) {
        auto& entry = material.second;
        auto& index = material.first;

        shader->setUniform("materials[" + std::to_string(index) + "].albedo",    entry.albedo);
        shader->setUniform("materials[" + std::to_string(index) + "].params", Vector3(entry.roughness, entry.metallic, entry.ao));

        for (int i=0;i<MAX_TEXTURE;i++) {
            if (entry.texture[i] == nullptr || !entry.texture[i]->isvalid()) {
                shader->setUniform("isactive["+ std::to_string(i) + "]", static_cast<int>(false));
                continue;
            }

            entry.texture[i]->bind(GL_TEXTURE0 + i);

            shader->setUniform("textures["+ std::to_string(i) + "]", i);
            shader->setUniform("isactive["+ std::to_string(i) + "]", static_cast<int>(true));
        }
    }
    m_shadowbuffer->GetTexture()->bind(GL_TEXTURE0 + 6);
    shader->setUniform("gShadow", 6);
}

void BatchingManager::SetupLight(Shader *shader) {
    auto& lights = LightComponent::m_lights;

    shader->setUniform("m_lightcount", static_cast<int>(lights.size()));

    if (lights.empty()) return;
    
    for (u32 i=0;i<lights.size();i++) {
        const auto& m_light = lights[i];

        shader->setUniform("m_lights[" + std::to_string(i) + "].position", m_light->getposition());
        shader->setUniform("m_lights[" + std::to_string(i) + "].color", m_light->getcolor());
        shader->setUniform("m_lights[" + std::to_string(i) + "].attenuationConstant", m_light->getattenuationconstant());
        shader->setUniform("m_lights[" + std::to_string(i) + "].attenuationLinear", m_light->getattenuationlinear());
        shader->setUniform("m_lights[" + std::to_string(i) + "].attenuationQuadratic", m_light->getattenuationquadractc());
        shader->setUniform("m_lights[" + std::to_string(i) + "].type", m_light->gettype());
        shader->setUniform("m_lights[" + std::to_string(i) + "].direction", m_light->getforward());
        shader->setUniform("m_lights[" + std::to_string(i) + "].angleInnerCone", m_light->getinnecone());
        shader->setUniform("m_lights[" + std::to_string(i) + "].angleOuterCone", m_light->getoutercone());
        shader->setUniform("m_lights[" + std::to_string(i) + "].vp", m_light->getprojectionview());
    }
}

void BatchingManager::SetupEnableShaderModel(Shader *shader) {
    shader->RegisterUniform("u_boneMatrices", 100, Shader::eMAT4x4);
    shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
    shader->RegisterUniform("model", 1, Shader::eMAT4x4);
    shader->RegisterUniform("view", 1, Shader::eMAT4x4);
    shader->RegisterUniform("gEye", 1, Shader::eVECTOR3);

    /* light */
    shader->RegisterUniform("m_lightcount", 1, Shader::eINT);

    for (u32 i=0;i<10;i++) {
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].position", 1, Shader::eVECTOR3);
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].color", 1, Shader::eVECTOR4);
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].attenuationConstant", 1, Shader::eFLOAT);
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].attenuationLinear", 1, Shader::eFLOAT);
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].attenuationQuadratic", 1, Shader::eFLOAT);
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].type", 1, Shader::eFLOAT);
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].direction", 1, Shader::eFLOAT);
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].angleInnerCone", 1, Shader::eFLOAT);
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].angleOuterCone", 1, Shader::eFLOAT);
        shader->RegisterUniform("m_lights[" + std::to_string(i) + "].vp", 1, Shader::eMAT4x4);
    }

    /* material */
    for (u32 i=0;i<50;i++) {
        shader->RegisterUniform("materials[" + std::to_string(i) + "].albedo", 1, Shader::eVECTOR3);
        shader->RegisterUniform("materials[" + std::to_string(i) + "].params", 1, Shader::eVECTOR3);

        for (u32 j=0;j<MAX_TEXTURE;j++) {
            shader->RegisterUniform("isactive["+ std::to_string(j) + "]", static_cast<int>(false), Shader::eINT);
            shader->RegisterUniform("textures["+ std::to_string(j) + "]", 1, Shader::eINT);
            shader->RegisterUniform("isactive["+ std::to_string(j) + "]", static_cast<int>(true), Shader::eINT);
        }
    }
    shader->RegisterUniform("gShadow", 1, Shader::eINT);
    shader->RegisterUniform("isanimation", 1, Shader::eINT);
}

void BatchingManager::SetupEnableShaderLight(Shader *shader) {
    shader->RegisterUniform("gPosition", 1, Shader::eINT);
    shader->RegisterUniform("gNormal", 1, Shader::eINT);
    shader->RegisterUniform("gAlbedo", 1, Shader::eINT);
    shader->RegisterUniform("gRoughness", 1, Shader::eINT);
    shader->RegisterUniform("gAo", 1, Shader::eINT);
    shader->RegisterUniform("gMetallic", 1, Shader::eINT);
    // shader->RegisterUniform("gShadow", 1, Shader::eINT);

    shader->RegisterUniform("gEye", 1, Shader::eVECTOR3);
}

void BatchingManager::SetupEnableShaderShadow(Shader *shader) {
    shader->RegisterUniform("model", 1, Shader::eMAT4x4);
    shader->RegisterUniform("vp", 1, Shader::eMAT4x4);

    shader->RegisterUniform("u_boneMatrices", 100, Shader::eMAT4x4);
    shader->RegisterUniform("isanimation", 1, Shader::eINT);
}
