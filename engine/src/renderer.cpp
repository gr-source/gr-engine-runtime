#include "renderer.h"

#include "geometry/mesh.h"

#include "resources/Resources.h"

#include "component/cameraComponent.h"


std::unique_ptr<BufferManager> DebugDraw::m_primitivebuffer = nullptr;

std::unique_ptr<BufferManager> DebugDraw::m_spritebuffer = nullptr;

std::vector<SpriteRender> DebugDraw::m_sprites;

std::vector<Line> DebugDraw::m_lines;

std::vector<Line> DebugDraw::m_points;

Shader* DebugDraw::m_spriteRender = nullptr;

Shader* DebugDraw::m_shader = nullptr;

void DebugDraw::initialize() {
    m_spriteRender = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader) + "/SpriteRender", DebugDraw::SetupSpriteRender);

    m_shader = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader) + "/ShaderPrimitive", DebugDraw::SetupShaderPrimitive);

    m_primitivebuffer = std::make_unique<BufferManager>(BufferManager::eDYNAMIC);

    m_primitivebuffer->bind(BufferManager::eVAO);

    m_primitivebuffer->bind(BufferManager::eVBO);
    m_primitivebuffer->attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Line), reinterpret_cast<void*>(offsetof(Line, position)));
    m_primitivebuffer->attribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Line), reinterpret_cast<void*>(offsetof(Line, color)));

    m_primitivebuffer->unbind();

    glPointSize(5.0f);
    glLineWidth(2.5f);
}

void DebugDraw::SetupSpriteRender(Shader* shader) {
    shader->RegisterUniform("m_texture", 1, Shader::eINT);

    shader->RegisterUniform("model", 1, Shader::eMAT4x4);
    shader->RegisterUniform("vp", 1, Shader::eMAT4x4);

    m_spritebuffer = std::make_unique<BufferManager>(BufferManager::eSTATIC, true);

    Mesh m_mesh;
    m_mesh.addvertex(Vector3(-1.0f, 0.0f,  1.0f), Vector3::zero, Vector2(1.0f, 0.0f));
    m_mesh.addvertex(Vector3( 1.0f, 0.0f,  1.0f), Vector3::zero, Vector2(0.0f, 1.0f));
    m_mesh.addvertex(Vector3(-1.0f, 0.0f, -1.0f), Vector3::zero, Vector2(0.0f, 0.0f));
    m_mesh.addvertex(Vector3( 1.0f, 0.0f, -1.0f), Vector3::zero, Vector2(1.0f, 1.0f));

    m_mesh.addtriangle(1, 2, 0);
    m_mesh.addtriangle(1, 3, 2);

    m_spritebuffer->bind(BufferManager::eVAO);

    m_spritebuffer->bind(BufferManager::eEBO);
    m_spritebuffer->pushElements(m_mesh.getindices().size() * sizeof(GLuint), m_mesh.getindices().data());

    m_spritebuffer->bind(BufferManager::eVBO);
    m_spritebuffer->pushBuffer(m_mesh.getvertices().size() * sizeof(Vertex3D), m_mesh.getvertices().data());

    m_spritebuffer->attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, position)));
    m_spritebuffer->attribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, uv)));

    m_spritebuffer->unbind();
}

void DebugDraw::SetupShaderPrimitive(Shader *shader) {
    shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
    shader->RegisterUniform("view", 1, Shader::eMAT4x4);
}

void DebugDraw::addLine(const Vector3& start, const Vector3& end, const Color& color) {
    m_lines.emplace_back(start, color);
    m_lines.emplace_back(end, color);
}

void DebugDraw::addPoint(const Vector3 &position, const Color &color) {
    m_points.emplace_back(position, color);
}

void DebugDraw::addSprite(const Vector3 &position, Sprite *sprite) {
    CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    Quaternion rotate = Math::normalize(Math::lookRotation(Math::normalize(position - m_cameraComponent.getposition()), Vector3::up));
    rotate *= Math::normalize(Quaternion::Euler(Math::rad(90.0f), Vector3(1.0f, 0.0f, 0.0f)));
    rotate = Math::normalize(rotate);

    const Matrix4x4 model = Math::translate(position) * Math::rotate(rotate) * Math::scale(Vector3::one);
    m_sprites.push_back({sprite, model});
}

const bool DebugDraw::isvalid() {
    return m_shader->isvalid();
}

const bool DebugDraw::hasLines() {
    return !m_lines.empty();
}

void DebugDraw::bind() {
    CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    m_shader->bind();

    m_shader->setUniform("projection", m_cameraComponent.getperspesctive());
    m_shader->setUniform("view", m_cameraComponent.getviewmatrix());
}

void DebugDraw::unbind() {
    m_shader->unbind();
    m_lines.clear();
    m_points.clear();
    m_sprites.clear();
}

void DebugDraw::Renderer() {
    if (!m_shader->isvalid()) return;

    DebugDraw::bind();
    if (DebugDraw::hasLines()) {
        DebugDraw::drawLines();
    }
    if (DebugDraw::hasPoints()) {
        DebugDraw::drawPoints();
    }
    if (DebugDraw::hassprites()) {
        DebugDraw::drawsprites();
    }
    DebugDraw::unbind();
}

void DebugDraw::drawLines() {
    m_primitivebuffer->bind(BufferManager::eVAO);

    m_primitivebuffer->bind(BufferManager::eVBO);
    m_primitivebuffer->pushBuffer(m_lines.size() * sizeof(Line), m_lines.data());

    m_primitivebuffer->draw(BufferManager::eLine, 0, static_cast<GLsizei>(m_lines.size()));

    m_primitivebuffer->unbind();
}

const bool DebugDraw::hasPoints() {
    return !m_points.empty();
}

void DebugDraw::drawPoints() {
    m_primitivebuffer->bind(BufferManager::eVAO);

    m_primitivebuffer->bind(BufferManager::eVBO);
    m_primitivebuffer->pushBuffer(m_points.size() * sizeof(Line), m_points.data());

    m_primitivebuffer->draw(BufferManager::ePoint, 0, static_cast<GLsizei>(m_points.size()));

    m_primitivebuffer->unbind();
}

const bool DebugDraw::hassprites() {
    return !m_sprites.empty() && m_spritebuffer != nullptr;
}

void DebugDraw::drawsprites() {
    CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    m_spriteRender->bind();

    m_spritebuffer->bind(BufferManager::eVAO);

    m_spriteRender->setUniform("vp", m_cameraComponent.getperspesctive() * m_cameraComponent.getviewmatrix());

    for (auto& entry : m_sprites) {
        m_spriteRender->setUniform("model", entry.model);

        entry.m_sprite->bind(GL_TEXTURE0);
        m_spriteRender->setUniform("m_texture", 0);

        m_spritebuffer->draw(BufferManager::eTriangle, 6, BufferManager::eUint, nullptr);
    }

    m_spritebuffer->unbind();

    m_spriteRender->unbind();
}
