#include "renderManager.h"
#include "gr-base/core/culling/octreemanager.hpp"
#include "gr-base/core/geometry/shader.hpp"

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

#include <gr-base/core/geometry/batchingmanager.hpp>

#include <gr-base/core/culling/frustum.hpp>


#include <gr-base/ecs/gameObject.hpp>

void RenderManager::Process(const Camera *camera)
{
    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();
    auto position = camera->getPosition();

    auto list = AssetsManager::GetShaderList(ShaderType::SHADER3D);
    for (auto &shader : list)
    {
        if (!shader || !shader->isValid())
        {
            continue;
        }

        shader->setUniform("u_projection", &projection);
        shader->setUniform("u_view",       &view);
        shader->setUniform("u_eye",        &position);

        SetupLight(shader);
    }

    Frustum::CalculateFrustum(projection * view);

    // OctreeManager::Foreach(&RenderManager::CheckNodeOnCamFrustum);
}

void RenderManager::Renderer()
{
    BatchingManager::Render();
}

bool RenderManager::CheckNodeOnCamFrustum(OctreeNode *node)
{
    Vector3 position = node->position;
    float radius = node->radius;

    Vector3 min = position - Vector3{radius, radius, radius};
    Vector3 max = position + Vector3{radius, radius, radius};

    if (!Frustum::IsCubeInFrustumAABB(min, max))
        return true;
    
    for (int i=0;i<node->count;i++)
    {
        nquery &query = node->data[i];

        /*
        GameObject *gameObject = query.transform->getGameObject();
        if (gameObject->hasComponent<MeshRender>())
        {
            MeshRender *meshRender = gameObject->getComponent<MeshRender>();

            if (meshRender->batchingdynamic != nullptr)
                meshRender->batchingdynamic->insert(query.transform);
        }
        */
    }
    return false;
}

void RenderManager::SetupLight(grr::gShader *shader)
{
    auto lights = Light::GetListLight();

    int numLight = static_cast<int>(lights.size());

    if (!shader->setUniform("numLights", &numLight))
    {
        return;
    }

    if (lights.empty()) return;

    for (std::size_t i={};i<lights.size();i++)
    {
        auto &light = lights[i];

        auto position   = light->getPosition();
        auto forward    = light->getForward();
        auto color      = light->getColor();
        auto type       = light->getType();

        // auto attenuation = Vector3{light->getAttenuationConstant(), light->getAttenuationLinear(), light->getAttenuationQuadractc()};
        // auto params      = Vector3{light->getInnecone(), light->getOutercone(), (float)light->getType()};
        // auto projection  = light->getProjetion();
        
        shader->setUniform(("lights[" + std::to_string(i) + "].position").c_str(),    &position);
        shader->setUniform(("lights[" + std::to_string(i) + "].direction").c_str(),   &forward);
        shader->setUniform(("lights[" + std::to_string(i) + "].color").c_str(),       &color);
        // shader->setUniform(("lights[" + std::to_string(i) + "].attenuation").c_str(), &attenuation);
        // shader->setUniform(("lights[" + std::to_string(i) + "].params").c_str(),      &params);
        shader->setUniform(("lights[" + std::to_string(i) + "].type").c_str(),       &type);
    }
}





















