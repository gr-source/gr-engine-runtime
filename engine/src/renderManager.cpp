#include "renderManager.h"

#include "console.hpp"

#include "script/meshRender.hpp"
#include "script/meshFilter.hpp"
#include "script/transform.hpp"
#include "script/animator.hpp"
#include "script/camera.hpp"
#include "script/light.hpp"

#include "core/file/file.hpp"

#include "core/assetsManager.hpp"

#include "scriptManager/scriptManager.hpp"

#include "geometry/batching.hpp"

#include "culling/frustum.h"

#include <cstring>

#include <gr-render/gTexture.h>

std::array<grr::gShader *, 4> RenderManager::m_shader = {nullptr, nullptr, nullptr, nullptr};

bool RenderManager::bWireFrame = false;

Camera *RenderManager::m_camera = nullptr;

bool RenderManager::bRender = false;

void RenderManager::Initialize() {
    AssetsManager::LoadShader("dynamic_one_model",
        File::GetAsset("shader/model/dynamic_model.frag"),
        File::GetAsset("shader/model/dynamic_one_model.vert"),
        &RenderManager::SetupShader, ShaderType_Dynamic
    );
    
    AssetsManager::LoadShader("dynamic_anim",
        File::GetAsset("shader/model/dynamic_model.frag"),
        File::GetAsset("shader/model/dynamic_model_anim.vert"),
        &RenderManager::SetupShader, ShaderType_Dynamic_Anim
    );
    
    AssetsManager::LoadShader("dynamic_multi_model",
        File::GetAsset("shader/model/dynamic_model.frag"),
        File::GetAsset("shader/model/dynamic_multi_model.vert"),
        &RenderManager::SetupShader, ShaderType_Dynamic_Multi_Model
    );
    
    AssetsManager::LoadShader("static_multi_model",
        File::GetAsset("shader/model/dynamic_model.frag"),
        File::GetAsset("shader/model/static_multi_model.vert"),
        &RenderManager::SetupShader, ShaderType_Static_Multi_Model
    );
}

void RenderManager::Process() {
    if (!Camera::Current) {
        return;
    }
    m_camera = Camera::Current();
    if (!bRender || !m_camera) {
        return;
    }

    for (auto &shader : m_shader) {
        if (!shader || !shader->isValid()) {
            continue;
        }

        grr::gShader::Bind(shader);

        grr::gShader::SetUniform("u_projection", 1, m_camera->getPerspesctive());
        grr::gShader::SetUniform("u_view", 1, m_camera->getViewmatrix());
        grr::gShader::SetUniform("m_eye", 1, m_camera->getPosition());

        SetupLight();
    }
    // Frustum::CalculateFrustum(m_camera->getPerspesctive() * m_camera->getViewmatrix());
}

void RenderManager::Renderer() {
    if (!bRender || !m_camera) {
        return;
    }

    
    Batching::Renderer();

    for (int i=0;i<10;i++) {
        grr::gTexture::Bind(nullptr, i);
    }
}

void RenderManager::Shutdown() {
    m_camera = nullptr;
    
    for (auto &shader : m_shader) {
        shader = nullptr;
    }
}

void RenderManager::SetupLight() {
    if (!ScriptManager::GetListLight) {
        return;
    }
    auto& lights = ScriptManager::GetListLight();

    grr::gShader::SetUniform("m_lightcount", static_cast<int>(lights.size()));

    if (lights.empty()) return;

    for (auto i=0;i<lights.size();i++) {
        const auto& light = lights[i];

        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].position",    light->getposition());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].color",       light->getcolor());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].attenuation", Vector3(light->getattenuationconstant(), light->getattenuationlinear(), light->getattenuationquadractc()));
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].direction",   light->getForward());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].params",      Vector3(light->getinnecone(), light->getoutercone(), (float)light->getType()));
    }
}

void RenderManager::SetupShader(ShaderType type, grr::gShader *shader) {
    m_shader[type] = shader;

    grr::gShader::Bind(shader);

    RegisterUniform(type == ShaderType_Dynamic);

    grr::gShader::Unbind();

    bRender = true;
}

void RenderManager::RegisterUniform(bool model) {
    grr::gShader::Register("u_projection");
    grr::gShader::Register("u_view");

    if (model)
        grr::gShader::Register("u_model");
    // grr::gShader::Register("u_boneMatrices");
    // grr::gShader::Register("u_animatior");

    // FragmentShader
    grr::gShader::Register("m_eye");
    grr::gShader::Register("m_lightcount");

    grr::gShader::Register("m_texture");
    grr::gShader::Register("m_activate");
    
    for (auto i=0;i<10;i++) {
        // Material
        grr::gShader::Register("m_material[" + std::to_string(i) + "].albedo");
        grr::gShader::Register("m_material[" + std::to_string(i) + "].params");

        // Light
        grr::gShader::Register("m_light[" + std::to_string(i) + "].position");
        grr::gShader::Register("m_light[" + std::to_string(i) + "].color");
        grr::gShader::Register("m_light[" + std::to_string(i) + "].attenuation");
        grr::gShader::Register("m_light[" + std::to_string(i) + "].direction");
        grr::gShader::Register("m_light[" + std::to_string(i) + "].params");
        grr::gShader::Register("m_light[" + std::to_string(i) + "].vp");
    }
}
