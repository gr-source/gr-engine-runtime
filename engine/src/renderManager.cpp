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

void RenderManager::Process(const Camera *camera)
{
    if (!camera)
    {
        return;
    }

    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();
    auto position = camera->getPosition();

    for (auto shader : AssetsManager::GetShaderList())
    {
        if (!shader || !shader->isValid())
        {
            continue;
        }

        shader->setUniform("u_projection", &projection);
        shader->setUniform("u_view",       &view);
        shader->setUniform("m_eye",        &position);
    }

    Frustum::CalculateFrustum(projection * view);
}

void RenderManager::Renderer() {
    Batching::Render();

    for (int i=0;i<10;i++) {
        grr::gTexture::Unbind(i);
    }
}


void RenderManager::SetupLight(grr::gShader *shader) {
    auto lights = Light::GetListLight();

    int numLight = static_cast<int>(lights.size());

    shader->setUniform("m_lightcount", &numLight);

    if (lights.empty()) return;

    for (auto i=0;i<lights.size();i++) {
        auto light = lights[i];

        auto position    = light->getPosition();
        auto color       = light->getColor();
        auto attenuation = Vector3{light->getAttenuationConstant(), light->getAttenuationLinear(), light->getAttenuationQuadractc()};
        auto forward     = light->getForward();
        auto params      = Vector3{light->getInnecone(), light->getOutercone(), (float)light->getType()};
        auto projection  = light->getProjetion();

        shader->setUniform(("m_light[" + std::to_string(i) + "].position").c_str(),    &position);
        shader->setUniform(("m_light[" + std::to_string(i) + "].color").c_str(),       &color);
        shader->setUniform(("m_light[" + std::to_string(i) + "].attenuation").c_str(), &attenuation);
        shader->setUniform(("m_light[" + std::to_string(i) + "].direction").c_str(),   &forward);
        shader->setUniform(("m_light[" + std::to_string(i) + "].params").c_str(),      &params);
        shader->setUniform(("m_light[" + std::to_string(i) + "].model").c_str(),       &projection);
    }
}





















