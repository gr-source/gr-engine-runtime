#include "skybox.h"

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

Skybox::Skybox() : play(false), currentTime(0.0f), distance(400.0f), m_backgroundShader(Resources::Load<Shader>(Filesystem::GetFile("/shader") + "/Background", SetupBackgroundShader)), m_skyboxbuffer(std::make_unique<Skyboxbuffer>(512, 512)), m_skyboxShader(nullptr), m_vertexArray(nullptr), m_vbo(0), m_ebo(0) {
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

    m_texture = Resources::LoadTexture<void(Skybox::*)(Texture*texture), Skybox*>("assets/texture/skybox/industrial_sunset_puresky_1k.hdr", &Skybox::SetupBackgroundTexture, this);
}

void Skybox::update() {}

void Skybox::render() {
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
    m_skyboxShader = Resources::LoadShader(Filesystem::GetFile("/shader") + "/Skybox", &Skybox::setupSkybox, this);
}

void Skybox::setupSkybox(Shader* shader) {
    shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
    shader->RegisterUniform("view", 1, Shader::eMAT4x4);
    shader->RegisterUniform("environmentMap", 1, Shader::eINT);

    GenerateMap([&](u32 i) {
        m_skyboxbuffer->addSkyColor(Cubemap::ePOSITIVE_X + i, Framebuffer::eCOLOR_ATTACHMENT0);
    }, m_texture);

    m_skyboxShader = Resources::LoadShader(Filesystem::GetFile("/shader") + "/Irradiance", &Skybox::setupIrradiance, this);
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
