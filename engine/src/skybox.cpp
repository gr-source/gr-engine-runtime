#include "skybox.h"

#include "postProcessing.hpp"

/*
#include "timer.h"

#include "resources/resources.h"

#include "component/camera.hpp"
// #include "component/lightComponent.h"

#include <functional>

Skybox &Skybox::getInstance() {
    static Skybox m_skybox;
    return m_skybox;
}

void SetupBackgroundShader(Shader* shader) {
    shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
    shader->RegisterUniform("view", 1, Shader::eMAT4x4);
    shader->RegisterUniform("environmentMap", 1, Shader::eINT);
}

Skybox::Skybox() : play(false), currentTime(0.0f), distance(400.0f), m_skyboxbuffer(std::make_unique<Skyboxbuffer>(512, 512)), m_vertexArray(nullptr), m_vbo(0), m_ebo(0) {
    loadShader();

    captureProjection = Math::perspective(Math::rad(90.0f), 1.0f, 0.1f, 10.0f);
     
     captureViews = {
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 1.0f,  0.0f,  0.0f), Vector3(0.0f, -1.0f,  0.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(-1.0f,  0.0f,  0.0f), Vector3(0.0f, -1.0f,  0.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 0.0f,  1.0f,  0.0f), Vector3(0.0f,  0.0f,  1.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 0.0f, -1.0f,  0.0f), Vector3(0.0f,  0.0f, -1.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 0.0f,  0.0f,  1.0f), Vector3(0.0f, -1.0f,  0.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 0.0f,  0.0f, -1.0f), Vector3(0.0f, -1.0f,  0.0f))
    };

    // vertex array
    createVertexArray();

    // m_texture = Resources::LoadTexture<void(Skybox::*)(Texture*texture), Skybox*>("assets/texture/skybox/industrial_sunset_puresky_1k.hdr", &Skybox::SetupBackgroundTexture, this);
}

void Skybox::update() {}

void Skybox::render() {
    assert(m_shader || m_shader->isValid() && "Invalid shader");

    if (m_backgroundShader->isvalid()) {
        m_backgroundShader->bind();

        if (m_skyboxbuffer->getSkyColor()->isvalid()) {
            m_skyboxbuffer->getSkyColor()->bind();
            m_backgroundShader->setUniform("environmentMap", 0);
        }

        auto camera = Camera::Current();

        m_backgroundShader->setUniform("projection", camera->getPerspesctive());
        m_backgroundShader->setUniform("view", camera->getViewmatrix());

        m_vertexArray->bind();
        grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, count, nullptr);
        m_vertexArray->unbind();
    }

    m_backgroundShader->unbind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Cubemap *Skybox::getIrradiance() {
    return m_skyboxbuffer->getIrradiance();
}

void Skybox::loadShader() {
    auto frag = Filesystem::GetFileContent(Filesystem::GetFile("/shader/Background.frag"));
    auto vert = Filesystem::GetFileContent(Filesystem::GetFile("/shader/Background.vert"));

    auto fContent = frag.c_str();
    auto vContent = vert.c_str();

    m_shader = grr::gShader::Create(&fContent, &vContent);
}

void Skybox::createVertexArray() {
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

    count = indices.size();

    m_vbo = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    assert(m_vbo && "Falied to create vbo.");

    m_ebo = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO);
    assert(m_vbo && "Falied to create ebo.");

    m_vertexArray = grr::gVertexArray::Create();
    assert(m_vertexArray && "Falied to create vao.");

    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_ebo);
    grr::gVertexArray::UpdateResize(indices.size() * sizeof(u32));
    grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(u32), indices.data());

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResize(vertices.size() * sizeof(Vector3));
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vector3), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vector3), nullptr);

    m_vertexArray->unbind();
}

void Skybox::SetupBackgroundTexture(Texture *texture) {
    // m_skyboxShader = Resources::LoadShader(Filesystem::GetFile("/shader") + "/Skybox", &Skybox::setupSkybox, this);
}

void Skybox::setupSkybox(Shader* shader) {
    shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
    shader->RegisterUniform("view", 1, Shader::eMAT4x4);
    shader->RegisterUniform("environmentMap", 1, Shader::eINT);

    GenerateMap([&](u32 i) {
        m_skyboxbuffer->addSkyColor(Cubemap::ePOSITIVE_X + i, Framebuffer::eCOLOR_ATTACHMENT0);
    }, m_texture);

    // m_skyboxShader = Resources::LoadShader(Filesystem::GetFile("/shader") + "/Irradiance", &Skybox::setupIrradiance, this);
}

void Skybox::setupIrradiance(Shader *shader) {
    shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
    shader->RegisterUniform("view", 1, Shader::eMAT4x4);
    shader->RegisterUniform("environmentMap", 1, Shader::eINT);

    GenerateMap([&](u32 i) {
        m_skyboxbuffer->addIrradiance(Cubemap::ePOSITIVE_X + i, Framebuffer::eCOLOR_ATTACHMENT0);
    }, m_skyboxbuffer->getSkyColor());
}

void Skybox::GenerateMap(std::function<void(u32)> func, Texture* texture) {
    m_skyboxShader->bind();

    texture->bind();
    m_skyboxShader->setUniform("environmentMap", 0);

    m_skyboxShader->setUniform("projection", captureProjection);

    m_skyboxbuffer->bind();
    for (u32 i=0;i<captureViews.size();i++) {
        m_skyboxShader->setUniform("view", captureViews[i]);

        func(i);
        
        GRender(GRENDERER3D);

        m_vertexArray->bind();
        grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, count, nullptr);
        m_vertexArray->unbind();
    }
    m_skyboxbuffer->unbind();
    texture->unbind();
}
*/

#include "core/resources.h"

grr::gVertexArray *Skybox::m_vertexArray = nullptr;

grr::gShader *Skybox::m_irradianceShader = nullptr;

grr::gShader *Skybox::m_skyShader = nullptr;

grr::gShader *Skybox::m_shader = nullptr;

grm::u32 Skybox::m_count = 0;

grm::u32 Skybox::m_vbo = 0;

grm::u32 Skybox::m_ebo = 0;

void Skybox::Initialize() {
    CreateCube();

    m_irradianceShader = Resources::LoadShader(Filesystem::GetFile("/shader/skybox/Irradiance"));

    m_skyShader = Resources::LoadShader(Filesystem::GetFile("/shader/skybox/Skybox"));

    m_shader = Resources::LoadShader(Filesystem::GetFile("/shader/skybox/Background"));

    Resources::AddShaderLoaded(&Skybox::SetupShader);
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
    grr::gVertexArray::UpdateResize(indices.size() * sizeof(grm::u32));
    grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::u32), indices.data());

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResize(vertices.size() * sizeof(Vector3));
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vector3), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vector3), nullptr);

    m_vertexArray->unbind();
}

bool Skybox::SetupShader(grr::gShader *shader, const std::string &path) {
    if (shader == m_shader || shader == m_irradianceShader || shader == m_skyShader) {
        shader->bind();
        grr::gShader::Register("projection");
        grr::gShader::Register("u_texture");
        grr::gShader::Register("view");

        if (shader == m_skyShader) {
            // GenerateMap([&](u32 i) {
            //     m_skyboxbuffer->addSkyColor(Cubemap::ePOSITIVE_X + i, Framebuffer::eCOLOR_ATTACHMENT0);
            // }, m_texture);
        }

        auto isdelete = m_shader->isValid() && m_skyShader->isValid() && m_irradianceShader->isValid();

        return isdelete;
    }

    return false;
}

void Skybox::GenerateMap(std::function<void(grm::u32)> func, grr::gTexture *texture) {
    auto projection = Math::perspective(Math::rad(90.0f), 1.0f, 0.1f, 10.0f);
    
    grr::gTexture::Bind(texture, 0);
    grr::gShader::SetUniform("u_texture", 0);

    grr::gShader::SetUniform("projection", 1, projection);

    Matrix4x4 view[6]{
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
        m_vertexArray->unbind();
    }
    grr::gTexture::Bind(nullptr, 0);
}
