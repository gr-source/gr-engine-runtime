#include "renderManager.h"

#include "console.hpp"

#include "ecs/gameObject.h"

#include "component/meshRender.hpp"
#include "component/meshFilter.hpp"
#include "component/transform.hpp"
#include "component/camera.hpp"
#include "component/light.hpp"

#include "resources/resources.h"
#include "resources/shader.h"

#include "scriptManager/scriptManager.hpp"

#include "filesystem/filesystem.h"

std::vector<GameObject*> RenderManager::s_objects;

grr::gShader *RenderManager::m_shader = nullptr;

Frustum RenderManager::s_frustum;

bool RenderManager::bWireFrame = false;

int RenderManager::iDrawCount = 0;

int RenderManager::iDrawRender = 0;

void RenderManager::Initialize() {
    auto source = ScriptManager::GetShader()[0];

    const char *fragment = source.first.c_str();
    const char *vertex = source.second.c_str();

    Console::Print(LogInfo::INFO, "Render loading shader...");

    m_shader = grr::gShader::Create(&fragment, &vertex);
    if (!m_shader) {
        std::stringstream iss(grr::gError::GetError());

        std::string line;
        while (std::getline(iss, line)) {
            Console::Print(LogInfo::ERROR, line);
        }
        grr::gError::Clean();
        return;
    }
    m_shader->bind();

    // VertexShader
    grr::gShader::Register("projection");
    grr::gShader::Register("view");
    grr::gShader::Register("model");

    // FragmentShader
    grr::gShader::Register("m_eye");
    grr::gShader::Register("m_lightcount");

    grr::gShader::Register("m_texture");
    grr::gShader::Register("m_activate");
    
    for (auto i=0;i<10;i++) {
        // Textures
        // grr::gShader::Register("m_texture[" + std::to_string(i) + "]");
        
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

    Console::Print(LogInfo::INFO, "Shader loaded successfully.");
}

void RenderManager::Process() {
    auto& models = MeshRender::GetGameObjectFromRender();
    if (models.empty() || !m_shader || !m_shader->isValid()) {
        return;
    }

    s_objects.clear();

    iDrawCount = models.size();

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

    iDrawRender = s_objects.size();
}

void RenderManager::Renderer() {
    if (!m_shader || s_objects.empty() || !m_shader->isValid()) {
        return;
    }

    auto camera = Camera::Main();

    m_shader->bind();

    grr::gShader::SetUniform("projection", 1, camera->getPerspesctive());
    grr::gShader::SetUniform("view", 1, camera->getViewmatrix());
    grr::gShader::SetUniform("m_eye", 1, camera->getPosition());

    SetupLight();

    if (bWireFrame) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    for (auto& gameObject : s_objects) {
        auto& transform = gameObject->getComponent<Transform>();

        auto& meshrender = gameObject->getComponent<MeshRender>();

        SetupMaterial(meshrender.getMaterials());

        grr::gShader::SetUniform("model", 1, transform.getglobalmatrix());

        meshrender.draw();
    }

    for (int i=0;i<10;i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (bWireFrame) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void RenderManager::Shutdown() {
    if (m_shader) {
        delete m_shader;
    }

    s_objects.clear();

    m_shader = nullptr;
}

void RenderManager::SetupLight() {
    auto& lights = Light::GetListLight();

    grr::gShader::SetUniform("m_lightcount", (int)lights.size());

    if (lights.empty()) return;


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

void RenderManager::SetupMaterial(std::unordered_map<int, Material> &materials) {
    for (auto&& [index, material] : materials) {
        int idx[10]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        int idxEn[10]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        for (auto i=0;i<10;++i) {
            auto texture = material.getTexture(i);
            if (!texture || !texture->isValid()) {
                continue;
            }
            texture->bind(idx[i]);
            idxEn[i] = 1;
        }

        grr::gShader::SetUniform<int>("m_texture", 10, (const int *)idx);
        grr::gShader::SetUniform<int>("m_activate", 10, (const int *)idxEn);

        grr::gShader::SetUniform("m_material[" + std::to_string(index) + "].albedo", material.getAlbedo());
        grr::gShader::SetUniform("m_material[" + std::to_string(index) + "].params", Vector3(material.getRoughness(), material.getMetallic(), material.getAmbientOcclusion()));
    }
}

