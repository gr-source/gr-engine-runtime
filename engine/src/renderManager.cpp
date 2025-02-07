#include "renderManager.h"

#include "shadowMap.hpp"

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/transform.hpp>
#include <gr-base/script/animator.hpp>
#include <gr-base/script/camera.hpp>
#include <gr-base/script/light.hpp>

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/assetsManager.hpp>
#include <gr-base/core/console.hpp>

#include <gr-base/core/geometry/batching.hpp>

#include "scriptManager/scriptManager.hpp"

#include <gr-base/core/culling/frustum.hpp>

#include <cstring>

#include <gr-render/gTexture.h>
#include <gr-render/gError.h>

std::array<grr::gShader *, 4> RenderManager::m_shader = {nullptr, nullptr, nullptr, nullptr};

bool RenderManager::bWireFrame = false;

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
    if (!bRender || !camera) {
        return;
    }

    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();
    auto position = camera->getPosition();

    for (auto &shader : m_shader) {
        if (!shader || !shader->isValid()) {
            continue;
        }

        shader->setUniform("u_projection", 1, &projection);
        shader->setUniform("u_view",       1, &view);
        shader->setUniform("m_eye",        1, &position);

        SetupLight(shader);
    }
    Frustum::CalculateFrustum(camera->getPerspesctive() * camera->getViewmatrix());
}

void RenderManager::Renderer() {
    if (!bRender) { return; }

    Batching::RenderBatches();

    for (int i=0;i<10;i++) {
        grr::gTexture::Unbind(i);
    }
}

void RenderManager::Shutdown() {
    for (auto &shader : m_shader) {
        shader = nullptr;
    }
}

void RenderManager::SetupLight(grr::gShader *shader) {
    auto lights = Light::GetListLight();

    int numLight = static_cast<int>(lights.size());

    shader->setUniform("m_lightcount", 1, &numLight);

    if (lights.empty()) return;

    for (auto i=0;i<lights.size();i++) {
        auto light = lights[i];

        auto position    = light->getPosition();
        auto color       = light->getColor();
        auto attenuation = Vector3{light->getAttenuationConstant(), light->getAttenuationLinear(), light->getAttenuationQuadractc()};
        auto forward     = light->getForward();
        auto params      = Vector3{light->getInnecone(), light->getOutercone(), (float)light->getType()};
        auto projection  = light->getProjetion();

        shader->setUniform(("m_light[" + std::to_string(i) + "].position").c_str(),    1, &position);
        shader->setUniform(("m_light[" + std::to_string(i) + "].color").c_str(),       1, &color);
        shader->setUniform(("m_light[" + std::to_string(i) + "].attenuation").c_str(), 1, &attenuation);
        shader->setUniform(("m_light[" + std::to_string(i) + "].direction").c_str(),   1, &forward);
        shader->setUniform(("m_light[" + std::to_string(i) + "].params").c_str(),      1, &params);
        shader->setUniform(("m_light[" + std::to_string(i) + "].model").c_str(),       1, &projection);
    }
}

bool RenderManager::SetupShader(ShaderType type, grr::gShader *shader) {
    m_shader[type] = shader;

    RegisterUniform(type == ShaderType_Dynamic, shader);

    bRender = true;
    return true;
}

#define PRINT_ERROR(func) \
    if (func == -1) { \
        Console::LogError("[ RenderManager ] Fail to register uniform: %s", grr::error()); \
    }

void RenderManager::RegisterUniform(bool model, grr::gShader *shader) {
    PRINT_ERROR(shader->registry("u_projection", grr::UniformType_mat4));
    PRINT_ERROR(shader->registry("u_view",       grr::UniformType_mat4));

    if (model)
        PRINT_ERROR(shader->registry("u_model",    grr::UniformType_mat4));
    // PRINT_ERROR(shader->registry("u_boneMatrices", grr::UniformType_mat4));
    // PRINT_ERROR(shader->registry("u_animatior"));

    // FragmentShader
    PRINT_ERROR(shader->registry("m_eye",        grr::UniformType_vec3));
    // PRINT_ERROR(shader->registry("m_shadowMap",  grr::UniformType_int));
    PRINT_ERROR(shader->registry("m_lightcount", grr::UniformType_int));

    PRINT_ERROR(shader->registry("m_texture",  grr::UniformType_int));
    PRINT_ERROR(shader->registry("m_activate", grr::UniformType_int));

    char buff[256] = {0};
    
    for (int i=0;i<10;i++) {
        // Material
        sprintf(buff, "m_material[%d].albedo", i);
        PRINT_ERROR(shader->registry(buff, grr::UniformType_vec4));

        sprintf(buff, "m_material[%d].params", i);
        PRINT_ERROR(shader->registry(buff, grr::UniformType_vec3));

        // Light
        PRINT_ERROR(shader->registry(("m_light[" + std::to_string(i) + "].position").c_str(),    grr::UniformType_vec3));
        PRINT_ERROR(shader->registry(("m_light[" + std::to_string(i) + "].color").c_str(),       grr::UniformType_vec4));
        PRINT_ERROR(shader->registry(("m_light[" + std::to_string(i) + "].attenuation").c_str(), grr::UniformType_vec3));
        PRINT_ERROR(shader->registry(("m_light[" + std::to_string(i) + "].direction").c_str(),   grr::UniformType_vec3));
        PRINT_ERROR(shader->registry(("m_light[" + std::to_string(i) + "].params").c_str(),      grr::UniformType_vec3));
        PRINT_ERROR(shader->registry(("m_light[" + std::to_string(i) + "].model").c_str(),       grr::UniformType_mat4));
    }
}
