#include "skybox.h"

#include "bufferManager.h"
#include "timer.h"

#include "resources/resources.h"

#include "component/cameraComponent.h"
#include "component/lightComponent.h"

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

Skybox::Skybox() : play(false), currentTime(0.0f), distance(400.0f), m_backgroundShader(Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader).string() + "/Background", SetupBackgroundShader)), m_skyboxbuffer(std::make_unique<Skyboxbuffer>(512, 512)), m_skyboxShader(nullptr) {
     captureProjection = Math::perspective(Math::rad(90.0f), 1.0f, 0.1f, 10.0f);
     
     captureViews = {
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 1.0f,  0.0f,  0.0f), Vector3(0.0f, -1.0f,  0.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(-1.0f,  0.0f,  0.0f), Vector3(0.0f, -1.0f,  0.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 0.0f,  1.0f,  0.0f), Vector3(0.0f,  0.0f,  1.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 0.0f, -1.0f,  0.0f), Vector3(0.0f,  0.0f, -1.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 0.0f,  0.0f,  1.0f), Vector3(0.0f, -1.0f,  0.0f)),
        Math::lookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3( 0.0f,  0.0f, -1.0f), Vector3(0.0f, -1.0f,  0.0f))
    };

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

    std::vector<GLuint> indices = {
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

    bufferManager = std::make_unique<BufferManager>(BufferManager::eSTATIC, true);

    bufferManager->bind(BufferManager::eVAO);

    bufferManager->bind(BufferManager::eEBO);
    bufferManager->pushElements(indices.size() * sizeof(GLuint), indices.data());

    bufferManager->bind(BufferManager::eVBO);
    bufferManager->pushBuffer(vertices.size() * sizeof(Vector3), vertices.data());

    bufferManager->attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3));
    bufferManager->unbind();

    TextureArgs args {Texture::eRGBA16F, Texture::eRGBA, Texture::eFLOAT, Texture::eBILINEAR, false};

    m_texture = Resources::Load<void (Skybox::*)(Texture *texture), Skybox *>("/home/grsource/Downloads/lonely_road_afternoon_puresky_1k.hdr", &Skybox::SetupBackgroundTexture, this);

    /*m_texture = Resources::Load("/home/grsource/Downloads/lonely_road_afternoon_puresky_1k.hdr", [&](Texture* textulre) {
        m_skyboxShader = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader) + "/Skybox", [&](Shader* shader) {
            shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
            shader->RegisterUniform("view", 1, Shader::eMAT4x4);
            shader->RegisterUniform("environmentMap", 1, Shader::eINT);

            GenerateMap([&](u32 i) {
                m_skyboxbuffer->addSkyColor(Cubemap::ePOSITIVE_X + i, Framebuffer::eCOLOR_ATTACHMENT0);
            }, textulre);

            m_skyboxShader = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader) + "/Irradiance", [&](Shader* shader) {
                shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
                shader->RegisterUniform("view", 1, Shader::eMAT4x4);
                shader->RegisterUniform("environmentMap", 1, Shader::eINT);

                GenerateMap([&](u32 i) {
                    m_skyboxbuffer->addIrradiance(Cubemap::ePOSITIVE_X + i, Framebuffer::eCOLOR_ATTACHMENT0);
                }, m_skyboxbuffer->getSkyColor());
            });
        });
    }, Texture::eSRBG, Texture::eRGB, Texture::eNONE, Texture::eBILINEAR, false);
    */
}

void Skybox::update() {}

void Skybox::render() { return;
    if (m_backgroundShader->isvalid()) {
        m_backgroundShader->bind();

        if (m_skyboxbuffer->getSkyColor()->isvalid()) {
            m_skyboxbuffer->getSkyColor()->bind();
            m_backgroundShader->setUniform("environmentMap", 0);
        }

        CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

        m_backgroundShader->setUniform("projection", m_cameraComponent.getperspesctive());
        m_backgroundShader->setUniform("view", m_cameraComponent.getviewmatrix());

        bufferManager->bind(BufferManager::eVAO);
        bufferManager->draw(BufferManager::eTriangle, count, BufferManager::eUint);
        bufferManager->unbind();
    }

    m_backgroundShader->unbind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Cubemap *Skybox::getIrradiance() {
    return m_skyboxbuffer->getIrradiance();
}

void Skybox::SetupBackgroundTexture(Texture *texture) {
    std::cout << "Background texture is: " << texture->getpath() << std::endl;
}

void Skybox::prepareShader()
{
}

void Skybox::setupSkybox(Shader* shader) {
    
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

        bufferManager->bind(BufferManager::eVAO);
        bufferManager->draw(BufferManager::eTriangle, count, BufferManager::eUint);
    }
    m_skyboxbuffer->unbind();
    texture->unbind();
}
