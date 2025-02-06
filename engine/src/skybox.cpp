#include "skybox.h"

#include <gr-base/core/assetsManager.hpp>
#include <gr-base/core/event.hpp>

#include <gr-base/script/camera.hpp>

#include "cubeFramebuffer.hpp"
#include "postProcessing.hpp"

#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

grr::gVertexArray *Skybox::m_vertexArray = nullptr;

grr::gShader *Skybox::m_irradianceShader = nullptr;

CubeFramebuffer *Skybox::m_cubeColor = nullptr;

grr::gShader *Skybox::m_skyShader = nullptr;

grr::gShader *Skybox::m_shader = nullptr;

grr::gTexture *Skybox::m_texture = nullptr;

grm::u32 Skybox::m_count = 0;

grm::u32 Skybox::m_vbo = 0;

grm::u32 Skybox::m_ebo = 0;

// TODO: falta adicionara a textura default ao skybox
void Skybox::Initialize() {
    m_cubeColor = CubeFramebuffer::Create({512, 512});

    CreateCube();

    AssetsManager::LoadTexture(File::GetAsset("/texture/skybox/industrial_sunset_puresky_1k.hdr", true), &Skybox::SetupTexture);
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

void Skybox::Renderer(Camera *camera) {
    if (!camera || !m_shader || !m_shader->isValid()) {
        return;
    }

    int textureID = 0;

    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();

    m_shader->bind();

    CubeFramebuffer::GetTextureColor(m_cubeColor)->bind(0);
    m_shader->setUniform("u_texture", 1, &textureID);

    m_shader->setUniform("projection", 1, &projection);
    m_shader->setUniform("view", 1, &view);

    m_shader->flush();
    
    m_vertexArray->bind();

    grr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_count, nullptr);
}

grr::gTexture *Skybox::GetTextureCubemap() {
    return CubeFramebuffer::GetTextureColor(m_cubeColor);
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

void Skybox::SetupTexture(grr::gTexture *texture) {
    m_texture = texture;

    Event::Add(&Skybox::UploadShader);
}

void SetCommonRegister(grr::gShader *shader) {
    shader->bind();
    shader->registry("projection", grr::UniformType_mat4);
    shader->registry("view",       grr::UniformType_mat4);
    shader->registry("u_texture",  grr::UniformType_int);
}

void RegisterDeafult(grr::gShader **out, grr::gShader *shader) {
    *out = shader;

    if (!shader) { return; }

    SetCommonRegister(shader);
}

void Skybox::RegisterSky(grr::gShader **out, grr::gShader *shader) {
    *out = shader;

    if (!shader) { return; }

    SetCommonRegister(shader);

    CubeFramebuffer::Bind(m_cubeColor);
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
            m_cubeColor->SetTexture(flags);
        }, m_texture, shader);
}

void Skybox::UploadShader() {
    AssetsManager::LoadShader("SkyboxIrradianceShader",
        File::GetAsset("/shader/skybox/Irradiance.frag"),
        File::GetAsset("/shader/skybox/Irradiance.vert"),
        RegisterDeafult, &m_irradianceShader
    );

    AssetsManager::LoadShader("SkyBoxShader",
        File::GetAsset("/shader/skybox/Skybox.frag"),
        File::GetAsset("/shader/skybox/Skybox.vert"),
        Skybox::RegisterSky, &m_skyShader
    );

    AssetsManager::LoadShader("SkyboxBackgroundShader",
        File::GetAsset("/shader/skybox/Background.frag"),
        File::GetAsset("/shader/skybox/Background.vert"),
        RegisterDeafult, &m_shader
    );
}

void Skybox::GenerateMap(std::function<void(grm::u32)> func, grr::gTexture *texture, grr::gShader *shader) {
    auto projection = Math::perspective(Math::rad(90.0f), 1.0f, 0.1f, 10.0f);

    int textureID = 0;

    texture->bind(textureID);
    shader->setUniform("u_texture", 1, &textureID);

    shader->setUniform("projection", 1, &projection);

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
        shader->setUniform("view", 1, &view[i]);

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
