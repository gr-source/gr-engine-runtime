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
#include <gr-base/core/clock.hpp>

#include <gr-base/core/geometry/batching.hpp>

#include "scriptManager/scriptManager.hpp"

#include <gr-base/core/culling/frustum.hpp>

#include <cstring>

#include <gr-render/gTexture.h>
#include <gr-render/gError.h>

#include "skybox.h"

bool RenderManager::bWireFrame = false;

void RenderManager::Initialize() {}

void RenderManager::Shutdown() {}

void RenderManager::Process(const Camera *camera, ShadowMap *shadowMap) {
    if (!camera) { return; }

    auto irradianceMap = Skybox::GetTextureIrradiance();

    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();
    auto position = camera->getPosition();

    static float TIME = 0.0f;
    TIME += Clock::deltaTime;
    TIME = std::fmod(TIME, 360.0f);

    int textureID = 4;

    for (auto &&[name, shader] : AssetsManager::GetShaderList()) {
        if (!shader || !shader->isValid()) {
            continue;
        }

        shader->setUniform("u_projection", 1, &projection);
        shader->setUniform("u_view",       1, &view);
        shader->setUniform("m_eye",        1, &position);
        shader->setUniform("TIME",         1, &TIME);
        
        if (irradianceMap != nullptr)
        {
            irradianceMap->bind(textureID);
            shader->setUniform("irradianceMap", 1, &textureID);
        }

        SetupLight(shader.get());
    }
    Frustum::CalculateFrustum(projection * view);
}

void RenderManager::Renderer() {
    Batching::RenderBatches();

    for (int i=0;i<10;i++) {
        grr::gTexture::Unbind(i);
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
