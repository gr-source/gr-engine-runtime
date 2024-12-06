#include "renderManager.h"

#include "shadowMap.hpp"

#include "script/meshRender.hpp"
#include "script/meshFilter.hpp"
#include "script/transform.hpp"
#include "script/animator.hpp"
#include "script/camera.hpp"
#include "script/light.hpp"

#include "core/file/file.hpp"

#include "core/assetsManager.hpp"
#include "core/console.hpp"

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
        File::GetAsset("/shader/model/dynamic_model.frag"),
        File::GetAsset("/shader/model/dynamic_one_model.vert"),
        &RenderManager::SetupShader, ShaderType_Dynamic
    );
    
    AssetsManager::LoadShader("dynamic_anim",
        File::GetAsset("/shader/model/dynamic_model.frag"),
        File::GetAsset("/shader/model/dynamic_model_anim.vert"),
        &RenderManager::SetupShader, ShaderType_Dynamic_Anim
    );
    
    AssetsManager::LoadShader("dynamic_multi_model",
        File::GetAsset("/shader/model/dynamic_model.frag"),
        File::GetAsset("/shader/model/dynamic_multi_model.vert"),
        &RenderManager::SetupShader, ShaderType_Dynamic_Multi_Model
    );
    
    AssetsManager::LoadShader("static_multi_model",
        File::GetAsset("/shader/model/dynamic_model.frag"),
        File::GetAsset("/shader/model/static_multi_model.vert"),
        &RenderManager::SetupShader, ShaderType_Static_Multi_Model
    );
}

void RenderManager::Process(Camera *camera, ShadowMap *shadowMap) {
    m_camera = camera;
    if (!bRender || !m_camera) {
        return;
    }

    for (auto &shader : m_shader) {
        if (!shader || !shader->isValid()) {
            continue;
        }

        grr::gShader::SetUniform("u_projection", m_camera->getPerspesctive());
        grr::gShader::SetUniform("u_view", m_camera->getViewmatrix());
        grr::gShader::SetUniform("m_eye", m_camera->getPosition());

        shadowMap->bindTexture(8);
        grr::gShader::SetUniform("m_shadowMap", 8);

        SetupLight();
    }
    // Frustum::CalculateFrustum(m_camera->getPerspesctive() * m_camera->getViewmatrix());
}

void RenderManager::Renderer() {
    if (!bRender || !m_camera) {
        return;
    }

    Batching::RenderBatches();

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

        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].position",    light->getPosition());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].color",       light->getColor());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].attenuation", Vector3{light->getAttenuationConstant(), light->getAttenuationLinear(), light->getAttenuationQuadractc()});
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].direction",   light->getForward());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].params",      Vector3{light->getInnecone(), light->getOutercone(), (float)light->getType()});
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].model", light->getProjetion());
    }
}

void RenderManager::SetupShader(ShaderType type, grr::gShader *shader) {
    m_shader[type] = shader;

    RegisterUniform(type == ShaderType_Dynamic);

    bRender = true;
}

void RenderManager::RegisterUniform(bool model) {
    grr::gShader::Register("u_projection", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    grr::gShader::Register("u_view", UniformType::MAT4, 1, Matrix4x4::identityMatrix);

    if (model)
        grr::gShader::Register("u_model", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    grr::gShader::Register<Matrix4x4>("u_boneMatrices", UniformType::MAT4, 100, Matrix4x4::identityMatrix);
    // grr::gShader::Register("u_animatior");

    // FragmentShader
    grr::gShader::Register("m_eye", UniformType::VEC3, 1, Vector3::zero);
    grr::gShader::Register("m_shadowMap", UniformType::INT, 1, 0);
    grr::gShader::Register("m_lightcount", UniformType::INT, 1, 0);

    int idx[10] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idxEn[10] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    grr::gShader::Register<int>("m_texture", UniformType::INT, 10, idxEn);
    grr::gShader::Register("m_activate", UniformType::INT, 10, 0);
    
    for (auto i=0;i<10;i++) {
        // Material
        grr::gShader::Register("m_material[" + std::to_string(i) + "].albedo", UniformType::VEC4, 1, Vector4::zero);
        grr::gShader::Register("m_material[" + std::to_string(i) + "].params", UniformType::VEC3, 1, Vector3::zero);

        // Light
        grr::gShader::Register("m_light[" + std::to_string(i) + "].position", UniformType::VEC3, 1, Vector3::zero);
        grr::gShader::Register("m_light[" + std::to_string(i) + "].color", UniformType::VEC4, 1, Vector4::zero);
        grr::gShader::Register("m_light[" + std::to_string(i) + "].attenuation", UniformType::VEC3, 1, Vector3::zero);
        grr::gShader::Register("m_light[" + std::to_string(i) + "].direction", UniformType::VEC3, 1, Vector3::zero);
        grr::gShader::Register("m_light[" + std::to_string(i) + "].params", UniformType::VEC3, 1, Vector3::zero);
        grr::gShader::Register("m_light[" + std::to_string(i) + "].model", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    }
}
