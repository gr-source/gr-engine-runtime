#include "skybox.h"

#include "core/assetsManager.hpp"
#include "core/event.hpp"

#include "cubeFramebuffer.hpp"
#include "postProcessing.hpp"

#include "script/camera.hpp"

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

    m_texture = AssetsManager::LoadTexture("/home/grsource/Documentos/AppProjects/gr-engine/bin/assets/texture/skybox/industrial_sunset_puresky_1k.hdr");

    AssetsManager::AddPostLoader(&Skybox::SetupTexture);
}

void Skybox::Shutdown() {
    if (m_vertexArray) {
        m_vertexArray->destroy();
    }

    if (m_vbo) {
        grr::gVertexArray::DeleteBuffer(m_vbo);
    }

    if (m_ebo) {
        grr::gVertexArray::DeleteBuffer(m_ebo);
    }
}

void Skybox::Renderer() {
    if (!Camera::Current) {
        return;
    }
    auto camera = Camera::Current();
    if (!camera || !m_shader || !m_shader->isValid()) {
        return;
    }

    grr::gShader::Bind(m_shader);

    grr::gTexture::Bind(CubeFramebuffer::GetTextureColor(m_cubeColor), 0);
    grr::gShader::SetUniform("u_texture", 0);

    grr::gShader::SetUniform("projection", 1, camera->getPerspesctive());
    grr::gShader::SetUniform("view", 1, camera->getViewmatrix());

    m_vertexArray->bind();
    grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_count, nullptr);
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

    m_vbo = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    assert(m_vbo && "Falied to create vbo.");

    m_ebo = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO);
    assert(m_vbo && "Falied to create ebo.");

    m_vertexArray = grr::gVertexArray::Create();
    assert(m_vertexArray && "Falied to create vao.");

    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_ebo);
    grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::u32), grr::gBufferFlags_Static);
    grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::u32), indices.data());

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vector3), grr::gBufferFlags_Static);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vector3), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vector3), nullptr);

    m_vertexArray->unbind();
}

bool Skybox::SetupTexture(void *texture, const std::string &path) {
    if (m_texture != texture) {
        return false;
    }

    Event::Add([]{
        m_irradianceShader = AssetsManager::LoadShader("SkyboxIrradianceShader",
            ("/home/grsource/Documentos/AppProjects/gr-engine/bin/assets/shader/skybox/Irradiance.frag"),
            ("/home/grsource/Documentos/AppProjects/gr-engine/bin/assets/shader/skybox/Irradiance.vert")
        );

        m_skyShader = AssetsManager::LoadShader("SkyBoxShader",
            ("/home/grsource/Documentos/AppProjects/gr-engine/bin/assets/shader/skybox/Skybox.frag"),
            ("/home/grsource/Documentos/AppProjects/gr-engine/bin/assets/shader/skybox/Skybox.vert"));

        m_shader = AssetsManager::LoadShader("SkyboxBackgroundShader",
            ("/home/grsource/Documentos/AppProjects/gr-engine/bin/assets/shader/skybox/Background.frag"),
            ("/home/grsource/Documentos/AppProjects/gr-engine/bin/assets/shader/skybox/Background.vert")
        );

        AssetsManager::AddPostLoader(&Skybox::SetupShader);
    });

    return true;
}

bool Skybox::SetupShader(void *shader, const std::string &path) {
    if (shader == m_shader || shader == m_irradianceShader || shader == m_skyShader) {
        grr::gShader::Bind((grr::gShader *)shader);
        grr::gShader::Register("projection");
        grr::gShader::Register("u_texture");
        grr::gShader::Register("view");

        if (shader == m_skyShader) {

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
            }, m_texture);
            grr::gShader::Unbind();
        }

        if (shader == m_shader) {
        }

        auto isdelete = m_shader->isValid() && m_skyShader->isValid() && m_irradianceShader->isValid();

        return isdelete;
    }

    return false;
}

void Skybox::GenerateMap(std::function<void(grm::u32)> func, grr::gTexture *texture) {
    auto projection = Math::perspective(Math::rad(90.0f), 1.0f, 0.1f, 10.0f);

    // if (texture)
    grr::gTexture::Bind(texture, 0);
    grr::gShader::SetUniform("u_texture", 0);

    grr::gShader::SetUniform("projection", 1, projection);

    Matrix4x4 view[6] {
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, -1.0f,  0.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, -1.0f,  0.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f,  0.0f,  1.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f,  0.0f, -1.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, -1.0f,  0.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, -1.0f,  0.0f})
    };

    for (auto i=0;i<6;i++) {
        grr::gShader::SetUniform("view", 1, view[i]);

        func(i);

        grr::gRender::SetRenderState(grr::GR_BACKGROUND_COLOR, (void *)&Color::blue);
        grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_COLOR_BUFFER);
        grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER);
        
        m_vertexArray->bind();
        grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_count, nullptr);
    }
    grr::gTexture::Bind(nullptr, 0);
}
