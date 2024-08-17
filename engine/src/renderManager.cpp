#include "renderManager.h"

#include "console.hpp"

#include "ecs/ecs.hpp"

#include "component/meshRender.hpp"
#include "component/meshFilter.hpp"
#include "component/transform.hpp"
#include "component/animator.hpp"
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
    if (!ScriptManager::GetShader) {
        return;
    }
    auto shaders = ScriptManager::GetShader();
    auto it = shaders.find(0);
    if (it == shaders.end()) {
        return Console::Print(LogInfo::ERROR, "Falied to find model shader 'id=0'");
    }

    auto &&[frag, vert] = (*it).second;

    const char *fragment = frag.c_str();
    const char *vertex = vert.c_str();

    Console::Print(LogInfo::INFO, "Model loading shader...");

    m_shader = grr::gShader::Create(&fragment, &vertex);
    if (!m_shader || !m_shader->isValid()) {
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
    grr::gShader::Register("u_projection");
    grr::gShader::Register("u_view");
    grr::gShader::Register("u_model");
    grr::gShader::Register("u_boneMatrices");
    grr::gShader::Register("u_animatior");

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
    s_objects.clear();
    iDrawCount = 0;

    if (!m_shader || !m_shader->isValid()) {
        return;
    }

    auto camera = Camera::Current();

    s_frustum.CalculateFrustum(camera->getPerspesctive() * camera->getViewmatrix());

    Ecs::for_each<MeshRender>(&RenderManager::Filter);

    iDrawRender = s_objects.size();
}

void RenderManager::Renderer() {
    if (!m_shader || s_objects.empty() || !m_shader->isValid()) {
        return;
    }

    auto camera = Camera::Current();

    m_shader->bind();

    grr::gShader::SetUniform("u_projection", 1, camera->getPerspesctive());
    grr::gShader::SetUniform("u_view", 1, camera->getViewmatrix());
    grr::gShader::SetUniform("m_eye", 1, camera->getPosition());

    SetupLight();

    if (bWireFrame) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    for (auto& gameObject : s_objects) {
        auto& transform = gameObject->getComponent<Transform>();

        auto& meshrender = gameObject->getComponent<MeshRender>();

        auto hasAnimation = gameObject->hasComponent<Animator>();

        if (hasAnimation) {
            auto &animator = gameObject->getComponent<Animator>();

            auto &metrices = animator.m_finalBoneMatrices;

            grr::gShader::SetUniform<Matrix4x4>("u_boneMatrices", metrices.size(), metrices.data());
        }
        grr::gShader::SetUniform("u_animatior", static_cast<int>(hasAnimation));

        SetupMaterial(meshrender.getMaterials());

        grr::gShader::SetUniform("u_model", 1, transform.getGlobalMatrix());

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

void RenderManager::Filter(GameObject *gameObject, MeshRender *meshRender) {
    auto& transform = gameObject->getTransform();

    auto& meshFilter = gameObject->getComponent<MeshFilter>();

    auto& model = transform.getGlobalMatrix();
    auto& boundbox = meshFilter.getBoundbox();

    ++iDrawCount;

    if (!s_frustum.IsCubeInFrustumOOBB(boundbox.min, boundbox.max, model)) {
        return;
    }

    if (gameObject->hasComponent<Animator>()) {
        gameObject->getComponent<Animator>().UpdateAnim();
    }
    s_objects.push_back(gameObject);
}

void RenderManager::SetupLight() {
    if (!ScriptManager::GetListLight) {
        return;
    }
    auto& lights = ScriptManager::GetListLight();

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
            grr::gTexture::Bind(texture, idx[i]);
            idxEn[i] = 1;
        }

        grr::gShader::SetUniform<int>("m_texture", 10, (const int *)idx);
        grr::gShader::SetUniform<int>("m_activate", 10, (const int *)idxEn);

        grr::gShader::SetUniform("m_material[" + std::to_string(index) + "].albedo", material.getAlbedo());
        grr::gShader::SetUniform("m_material[" + std::to_string(index) + "].params", Vector3(material.getRoughness(), material.getMetallic(), material.getAmbientOcclusion()));
    }
}

