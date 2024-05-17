#include "screenRender.h"

#include "bufferManager.h"

#include "geometry/vertex2D.h"

#include "resources/Resources.h"

void EnableShader(Shader* shader) {
    shader->RegisterUniform("screenTexture", 1, Shader::eINT);
}

ScreenRender::ScreenRender() : m_shader(Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader) + "/Screen", EnableShader)), bufferManager(std::make_unique<BufferManager>(BufferManager::eSTATIC)) {
    std::vector<Vertex2D> vertice = {
        {Vector2(-1.0f,  1.0f), Vector2(0.0f, 1.0f)},
        {Vector2(-1.0f, -1.0f), Vector2(0.0f, 0.0f)},
        {Vector2( 1.0f, -1.0f), Vector2(1.0f, 0.0f)},

        {Vector2(-1.0f,  1.0f), Vector2(0.0f, 1.0f)},
        {Vector2( 1.0f, -1.0f), Vector2(1.0f, 0.0f)},
        {Vector2( 1.0f,  1.0f), Vector2(1.0f, 1.0f)}
    };
    
    bufferManager->bind(BufferManager::eVAO);

    bufferManager->bind(BufferManager::eVBO);
    bufferManager->pushBuffer(vertice.size() * sizeof(Vertex2D), vertice.data());

    bufferManager->attribPointer(0, 2, sizeof(Vertex2D), reinterpret_cast<void*>(offsetof(Vertex2D, position)));
    bufferManager->attribPointer(1, 2, sizeof(Vertex2D), reinterpret_cast<void*>(offsetof(Vertex2D, uv)));

    bufferManager->unbind();
}

void ScreenRender::render(Texture2D* texture) {
    if (!texture->isvalid() || !m_shader->isvalid()) return;

    m_shader->bind();

    texture->bind();
    m_shader->setUniform("screenTexture", 0);

    bufferManager->bind(BufferManager::eVAO);

    bufferManager->draw(BufferManager::eTriangle, 0, 6);

    m_shader->unbind();
    
    bufferManager->unbind();
    texture->unbind();
}

