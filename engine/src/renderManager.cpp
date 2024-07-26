#include "renderManager.h"

#include "gameObject.h"

#include "component/lightComponent.h"
#include "component/meshRender.hpp"
#include "component/meshFilter.hpp"
#include "component/Transform.h"

#include "component/camera.hpp"

#include "resources/resources.h"
#include "resources/shader.h"

#include "scriptManager/scriptManager.hpp"

#include "filesystem/filesystem.h"

Frustum RenderManager::s_frustum;

std::vector<GameObject*> RenderManager::s_objects;

grr::gShader *RenderManager::m_shader = nullptr;

void RenderManager::Initialize() {
    auto vstr = Filesystem::getfilecontent("assets/shader/default.vert");
    auto fstr = Filesystem::getfilecontent("assets/shader/default.frag");

    m_shader = grr::gShader::Create({fstr.c_str()}, {vstr.c_str()});
    m_shader->bind();

    grr::gShader::Register("projection");
    grr::gShader::Register("view");
    grr::gShader::Register("model");

    m_shader->unbind();
}

void RenderManager::Process() {
    auto& models = MeshRender::GetGameObjectFromRender();
    if (models.empty()) {
        return;
    }

    s_objects.clear();

    auto camera = Camera::Main();

    s_frustum.CalculateFrustum(camera->getPerspesctive() * camera->getViewmatrix());

    for (const auto& gameObject : models) {
        auto& transform = gameObject->getTransform();

        auto& meshFilter = gameObject->getComponent<MeshFilter>();

        auto& model = transform.getglobalmatrix();
        auto& boundbox = meshFilter.getBoundbox();

        if (!s_frustum.IsCubeInFrustumOOBB(boundbox.min, boundbox.max, model)) {
            continue;
        }

        s_objects.push_back(gameObject);
    }
}

void RenderManager::Renderer() {
    if (s_objects.empty() || !m_shader || !m_shader->isValid()) {
        return;
    }

    auto camera = Camera::Main();

    m_shader->bind();

    grr::gShader::SetUniform("projection", 1, camera->getPerspesctive());
    grr::gShader::SetUniform("view", 1, camera->getViewmatrix());

    for (auto& gameObject : s_objects) {
        auto& transform = gameObject->getComponent<Transform>();

        auto& meshrender = gameObject->getComponent<MeshRender>();

        grr::gShader::SetUniform("model", 1, transform.getglobalmatrix());

        meshrender.draw();
    }

    // m_shader->unbind();
}

void RenderManager::Shutdown() {
    if (m_shader) {
        m_shader->destroy();
    }

    s_objects.clear();
}
