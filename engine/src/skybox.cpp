#include "skybox.h"

#include <gr-base/core/assetsManager.hpp>
#include <gr-base/core/event.hpp>

#include <gr-base/script/camera.hpp>

#include <gr-base/core/file/file.hpp>

#include "cubeFramebuffer.hpp"
#include "postProcessing.hpp"

#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

grr::gVertexArray *Skybox::m_vertexArray = nullptr;

grr::gShader *Skybox::m_irradianceShader = nullptr;

std::unique_ptr<CubeFramebuffer> Skybox::m_colorbuffer = nullptr;

std::unique_ptr<CubeFramebuffer> Skybox::m_irradiance = nullptr;

grr::gShader *Skybox::m_shader = nullptr;

grr::gTexture *Skybox::m_cubemapTexture = nullptr;

grm::u32 Skybox::m_count = 0;

grm::u32 Skybox::m_vbo = 0;

grm::u32 Skybox::m_ebo = 0;

// TODO: falta adicionara a textura default ao skybox
void Skybox::Initialize() {
    m_colorbuffer = std::make_unique<CubeFramebuffer>(iVector2{512, 512});

    m_irradiance = std::make_unique<CubeFramebuffer>(iVector2{32, 32});

    CreateCube();

    AssetsManager::LoadTexture("/home/felipy/Downloads/Skyboxes/SunsetSky.png", &Skybox::ConfigCubemap);
}

void Skybox::Shutdown() {
    if (m_vertexArray) {
        grr::gVertexArray::Destroy(m_vertexArray);
    }

    if (m_vbo) {
        grr::gVertexArray::DeleteBuffer(m_vbo);
    }

    if (m_ebo) {
        grr::gVertexArray::DeleteBuffer(m_ebo);
    }
    m_vertexArray = nullptr;
    m_vbo = 0;
    m_ebo = 0;
}

void Skybox::Renderer(const Camera *camera) {
    if (!m_shader || !m_shader->isValid()) {
        return;
    }

    int textureID = 0;

    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();

    m_colorbuffer->getCubemapTexture()->bind(0);
    m_shader->setUniform("u_texture", &textureID);
    
    m_shader->setUniform("u_projection", &projection);
    m_shader->setUniform("u_view", &view);
    
    m_shader->bind();

    m_shader->flush();
    
    m_vertexArray->bind();
    
    grr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_count, nullptr);
}

grr::gTexture *Skybox::GetTextureCubemap() {
    return m_colorbuffer->getCubemapTexture();
}

grr::gTexture *Skybox::GetTextureIrradiance()
{
    return &(*m_irradiance->getCubemapTexture());
}

void Skybox::CreateCube() {
    std::vector<Vector3> vertices = {
        { 1.0f,  1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f,  1.0f},
        { 1.0f, -1.0f,  1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        {-1.0f,  1.0f,  1.0f},
        {-1.0f, -1.0f,  1.0f}
    };

    std::vector<grm::u32> indices = {
        1, 0, 4,
        5, 1, 4,
        1, 3, 0,
        3, 2, 0,
        3, 7, 2,
        7, 6, 2,
        0, 2, 4,
        2, 6, 4,
        5, 7, 1,
        7, 3, 1,
        7, 5, 6,
        5, 4, 6
    };

    m_count = indices.size();

    m_vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
    assert(m_vbo && "Falied to create vbo.");

    m_ebo = grr::gVertexArray::CreateBuffer(BufferType::EBO);
    assert(m_vbo && "Falied to create ebo.");

    m_vertexArray = grr::gVertexArray::Create();
    assert(m_vertexArray && "Falied to create vao.");

    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_ebo);
    grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::u32), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::u32), indices.data());

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vector3), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vector3), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vector3), nullptr);

    m_vertexArray->unbind();
}

void RegisterDeafult(grr::gShader **out, grr::gShader *shader) {
    *out = shader;
}

void Skybox::ConfigCubemap(grr::gTexture *texture) {
    m_cubemapTexture = texture;

    AssetsManager::LoadShader(
        File::GetAsset("/shader/skybox/skybox.glsl"),
        &Skybox::CreateCubemapBackground
    );
}

void Skybox::CreateCubemapBackground(grr::gShader *shader) {
    m_colorbuffer->bind();
    GenerateMap([](grm::u32 i) {
        grr::gFramebufferFlags flags = grr::gFramebufferFlags_Cubemap;
        switch (i) {
            case 0: flags = grr::gFramebufferFlags_Cubemap_Positive_X; break;
            case 1: flags = grr::gFramebufferFlags_Cubemap_Negative_X; break;
            case 2: flags = grr::gFramebufferFlags_Cubemap_Positive_Y; break;
            case 3: flags = grr::gFramebufferFlags_Cubemap_Negative_Y; break;
            case 4: flags = grr::gFramebufferFlags_Cubemap_Positive_Z; break;
            case 5: flags = grr::gFramebufferFlags_Cubemap_Negative_Z; break;
        }
        m_colorbuffer->setTextureAttachment(flags);
    }, m_cubemapTexture, shader);

    AssetsManager::LoadShader(
        File::GetAsset("/shader/skybox/background.glsl"),
        RegisterDeafult, &m_shader
    );

    AssetsManager::LoadShader(
        File::GetAsset("/shader/skybox/irradiance.glsl"),
        &Skybox::CreateCubemapIrradiance
    );
}

void Skybox::CreateCubemapIrradiance(grr::gShader *shader)
{
    m_irradiance->bind();
    GenerateMap([](grm::u32 i) {
        grr::gFramebufferFlags flags = grr::gFramebufferFlags_Cubemap;
        switch (i) {
            case 0: flags = grr::gFramebufferFlags_Cubemap_Positive_X; break;
            case 1: flags = grr::gFramebufferFlags_Cubemap_Negative_X; break;
            case 2: flags = grr::gFramebufferFlags_Cubemap_Positive_Y; break;
            case 3: flags = grr::gFramebufferFlags_Cubemap_Negative_Y; break;
            case 4: flags = grr::gFramebufferFlags_Cubemap_Positive_Z; break;
            case 5: flags = grr::gFramebufferFlags_Cubemap_Negative_Z; break;
        }
        m_irradiance->setTextureAttachment(flags);
    }, m_colorbuffer->getCubemapTexture(), shader);
}

void Skybox::GenerateMap(std::function<void(grm::u32)> func, grr::gTexture *texture, grr::gShader *shader) {
    auto projection = Math::perspective(Math::rad(90.0f), 1.0f, 0.1f, 10.0f);

    // cubefraembuffer a ta usando id 0
    int textureID = 1;

    texture->bind(textureID);
    shader->setUniform("u_texture", &textureID);

    shader->setUniform("u_projection", &projection);

    Matrix4x4 view[6] {
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, -1.0f,  0.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, -1.0f,  0.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f,  0.0f,  1.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f,  0.0f, -1.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, -1.0f,  0.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, -1.0f,  0.0f})
    };

    shader->bind();

    for (auto i=0;i<6;i++) {
        shader->setUniform("u_view", &view[i]);

        func(i);

        grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void *)&Color::blue);
        grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER | grr::GR_DEPTH_BUFFER);
        
        shader->flush();
        
        m_vertexArray->bind();
        
        grr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_count, nullptr);
    }
    texture->unbind();
    shader->unbind();
}
