#include "renderManager.h"

#include "gr-base/core/culling/frustum.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/assetsManager.hpp"

#include "gr-base/script/light.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

#include "gr-base/broadphase/bvhtree.hpp"

#include <map>

// public
void RenderManager::PreRender(const Camera *camera)
{
    return GetInstance().preRender(camera);
}

void RenderManager::Render()
{
    return GetInstance().render();
}

void RenderManager::PosRender()
{
    return GetInstance().posRender();
}

// private
RenderManager &RenderManager::GetInstance()
{
    static RenderManager instance;
    return instance;
}

RenderManager::RenderManager()
{}

RenderManager::~RenderManager()
{}

void RenderManager::preRender(const Camera *camera)
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

        setuplight(shader);
    }

    Frustum::CalculateFrustum(projection * view);
    /*
    bvh::Tree::For_each([](struct bvh::Node<void *> *node)
    {
        auto boundary = node->boundary;

        if (!Frustum::IsCubeInFrustumAABB(boundary.min, boundary.max))
            return true;

        if (node->isLeaf())
        {
            GameObject *gameObject = (GameObject *)node->data;
            
            if (gameObject->hasComponent<MeshFilter>() && gameObject->hasComponent<MeshRender>())
            {
                auto meshRender = gameObject->getComponent<MeshRender>();
                auto meshFilter = gameObject->getComponent<MeshFilter>();
                auto transform = gameObject->getTransform();

                const auto& materials = meshRender->getMaterial();
                const auto& meshes = meshFilter->getMesh();
                const auto &modelMatrix = transform->getGlobalMatrix();

                size_t count = std::min(materials.size(), meshes.size());
                for (size_t i = 0; i < count; ++i)
                {
                    MaterialID materialID = materials[i];
                    MeshID meshID = meshes[i];

                    m_renderable[materialID][meshID].push_back(modelMatrix);
                }
            }
        }
        return false;
    });
    */
}

void RenderManager::posRender()
{}

void RenderManager::render()
{
    BatchingManager::Render();
}

void RenderManager::setuplight(grr::gShader *shader)
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

/*
#include "renderManager.h"
#include "gr-base/broadphase/bvhtree.hpp"
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

void RenderManager::PreRender(const Camera *camera)
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

    bvh::Tree::For_each(&RenderManager::CheckNodeOnCamFrustum);
}

void RenderManager::Render()
{
    BatchingManager::Render();
}

void RenderManager::PosRender()
{}

bool RenderManager::CheckNodeOnCamFrustum(struct bvh::Node<void *> *node)
{
    auto boundary = node->boundary;

    if (!Frustum::IsCubeInFrustumAABB(boundary.min, boundary.max))
        return true;

    if (node->isLeaf())
    {
        GameObject *gameObject = (GameObject *)node->data;

        if (gameObject->hasComponent<MeshRender>())
        {
            auto meshRender = gameObject->getComponent<MeshRender>();

            meshRender->build();
        }
    }
    return fals














*/

















