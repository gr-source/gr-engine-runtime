#include "renderManager.h"

#include "console.hpp"

#include "script/meshRender.hpp"
#include "script/meshFilter.hpp"
#include "script/transform.hpp"
#include "script/animator.hpp"
#include "script/camera.hpp"
#include "script/light.hpp"

#include "core/file/file.hpp"

#include "core/assetsManager.hpp"

#include "scriptManager/scriptManager.hpp"

#include "geometry/batching.hpp"

#include "culling/frustum.h"

#include <cstring>

#include <gr-render/gColor.h>

std::array<grr::gShader *, 4> RenderManager::m_shader = {nullptr, nullptr, nullptr, nullptr};

bool RenderManager::bWireFrame = false;

Camera *RenderManager::m_camera = nullptr;

bool RenderManager::bRender = false;

void RenderManager::Initialize() {
    if (!ScriptManager::GetShaders) {
        return;
    }

    auto shaders = ScriptManager::GetShaders();

    AssetsManager::AddPostLoader(RenderManager::SetupShader);

    m_shader[ShaderType_Dynamic] = shaders[ShaderType_Dynamic];
    // m_shader[ShaderType_Dynamic_Anim] = shaders[ShaderType_Dynamic_Anim];
    m_shader[ShaderType_Dynamic_Multi_Model] = shaders[ShaderType_Dynamic_Multi_Model];
    m_shader[ShaderType_Static_Multi_Model] = shaders[ShaderType_Static_Multi_Model];
}

void RenderManager::Process() {
    if (!Camera::Current) {
        return;
    }
    m_camera = Camera::Current();
    if (!bRender || !m_camera) {
        return;
    }

    for (auto &shader : m_shader) {
        if (!shader || !shader->isValid()) {
            continue;
        }

        grr::gShader::Bind(shader);

        grr::gShader::SetUniform("u_projection", 1, m_camera->getPerspesctive());
        grr::gShader::SetUniform("u_view", 1, m_camera->getViewmatrix());
        grr::gShader::SetUniform("m_eye", 1, m_camera->getPosition());

        SetupLight();
    }
    // Frustum::CalculateFrustum(m_camera->getPerspesctive() * m_camera->getViewmatrix());
}

void RenderManager::Renderer() {
    if (!bRender || !m_camera) {
        return;
    }

    // grr::gShader::Bind(m_shader[ShaderType_Static_Multi_Model]);

    // if (bWireFrame) {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // }

    // grr::gShader::SetUniform("u_animatior", static_cast<int>(false));
    // grr::gShader::SetUniform("u_model", 1, Matrix4x4::identityMatrix);

    Batching::Renderer();

    for (int i=0;i<10;i++) {
        grr::gTexture::Bind(nullptr, i);
    }

    // if (bWireFrame) {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // }
}

void RenderManager::Shutdown() {
    // bRender = false;
    
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

        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].position",    light->getposition());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].color",       grr::gColor(light->getcolor().r, light->getcolor().g, light->getcolor().b, light->getcolor().a));
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].attenuation", Vector3(light->getattenuationconstant(), light->getattenuationlinear(), light->getattenuationquadractc()));
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].direction",   light->getForward());
        grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].params",      Vector3(light->getinnecone(), light->getoutercone(), (float)light->getType()));
        // grr::gShader::SetUniform("m_light[" + std::to_string(i) + "].vp",            light->getprojectionview());
    }
}

bool RenderManager::SetupShader(void *ptr, const std::string &path) {
    auto shader = static_cast<grr::gShader *>(ptr);
    if (m_shader[ShaderType_Dynamic] != shader &&
        m_shader[ShaderType_Dynamic_Multi_Model] != shader &&
        m_shader[ShaderType_Static_Multi_Model] != shader) {
        return false;
    }

    grr::gShader::Bind(shader);

    RegisterUniform(m_shader[ShaderType_Dynamic] == shader);

    grr::gShader::Unbind();

    Console::log(LogInfo::INFO, "Shader loaded successfully.");

    bRender = m_shader[ShaderType_Dynamic]->isValid() &&
        m_shader[ShaderType_Dynamic_Multi_Model]->isValid() &&
        m_shader[ShaderType_Static_Multi_Model]->isValid();

    return bRender;
}

void RenderManager::RegisterUniform(bool model) {
    grr::gShader::Register("u_projection");
    grr::gShader::Register("u_view");

    if (model)
        grr::gShader::Register("u_model");
    // grr::gShader::Register("u_boneMatrices");
    // grr::gShader::Register("u_animatior");

    // FragmentShader
    grr::gShader::Register("m_eye");
    grr::gShader::Register("m_lightcount");

    grr::gShader::Register("m_texture");
    grr::gShader::Register("m_activate");
    
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
}
