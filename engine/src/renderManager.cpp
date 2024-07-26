#include "renderManager.h"

#include "gameObject.h"

#include "component/meshRender.hpp"
#include "component/meshFilter.hpp"
#include "component/Transform.h"
#include "component/light.hpp"

#include "component/camera.hpp"

#include "resources/resources.h"
#include "resources/shader.h"

#include "scriptManager/scriptManager.hpp"

#include "filesystem/filesystem.h"

Frustum RenderManager::s_frustum;

std::vector<GameObject*> RenderManager::s_objects;

grr::gShader *RenderManager::m_shader = nullptr;

void RenderManager::Initialize() {
    auto vstr = Filesystem::getfilecontent("assets/shader/model/default3D.vert");
    auto fstr = Filesystem::getfilecontent("assets/shader/model/default3D.frag");

    m_shader = grr::gShader::Create({fstr.c_str()}, {vstr.c_str()});
    m_shader->bind();

    // VertexShader
    grr::gShader::Register("projection");
    grr::gShader::Register("view");
    grr::gShader::Register("model");

    // FragmentShader
    grr::gShader::Register("m_eye");
    grr::gShader::Register("m_lightcount");

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
    grr::gShader::SetUniform("m_eye", 1, camera->getPosition());

    SetupLight();

    for (auto& gameObject : s_objects) {
        auto& transform = gameObject->getComponent<Transform>();

        auto& meshrender = gameObject->getComponent<MeshRender>();

        SetupMaterial(meshrender.getMaterials());

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

void RenderManager::SetupLight() {
    auto& lights = Light::GetListLight();

    if (lights.empty()) return;

    grr::gShader::SetUniform("m_lightcount", (int)lights.size());

    for (auto i=0;i<lights.size();i++) {
        const auto& light = lights[i];

        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].position",    light->getposition());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].color",       grr::gColor(light->getcolor().r, light->getcolor().g, light->getcolor().b, light->getcolor().a));
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].attenuation", Vector3(light->getattenuationconstant(), light->getattenuationlinear(), light->getattenuationquadractc()));
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].direction",   light->getforward());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].params",      Vector3(light->getinnecone(), light->getoutercone(), (float)light->gettype()));
        // grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].vp",            light->getprojectionview());
    }
}

void RenderManager::SetupMaterial(const std::unordered_map<int, Material> &materials) {
    for (const auto& [index, material] : materials) {

        grr::gShader::SetUniform("m_material[" + std::to_string(index) + "].albedo", material.getAlbedo());
        grr::gShader::SetUniform("m_material[" + std::to_string(index) + "].params", Vector3(material.getRoughness(), material.getMetallic(), material.getAmbientOcclusion()));
    }
}
