#include "renderer.h"

#include "geometry/mesh.h"

#include "resources/resources.h"

#include "component/camera.hpp"

std::unique_ptr<BufferManager> DebugDraw::m_primitivebuffer = nullptr;

std::unique_ptr<BufferManager> DebugDraw::m_spritebuffer = nullptr;

std::vector<Line> DebugDraw::m_lines;

std::vector<Line> DebugDraw::m_points;

Shader* DebugDraw::m_shader = nullptr;

bool DebugDraw::bDebugMode = false;

void DebugDraw::initialize() {
    m_shader = Resources::Load<Shader>(Filesystem::GetFile("/shader") + "/ShaderPrimitive", DebugDraw::SetupShaderPrimitive);

    m_primitivebuffer = std::make_unique<BufferManager>(BufferManager::eDYNAMIC);

    m_primitivebuffer->bind(BufferManager::eVAO);

    m_primitivebuffer->bind(BufferManager::eVBO);
    m_primitivebuffer->attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Line), reinterpret_cast<void*>(offsetof(Line, position)));
    m_primitivebuffer->attribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Line), reinterpret_cast<void*>(offsetof(Line, color)));

    m_primitivebuffer->unbind();
}

void DebugDraw::SetupShaderPrimitive(Shader *shader) {
    shader->RegisterUniform("projection", 1, Shader::eMAT4x4);
    shader->RegisterUniform("view", 1, Shader::eMAT4x4);
}

void DebugDraw::addLine(const Vector3& start, const Vector3& end, const Color& color) {
    if (!DebugDraw::bDebugMode) return;
    
    m_lines.emplace_back(start, color);
    m_lines.emplace_back(end, color);
}

void DebugDraw::addPoint(const Vector3 &position, const Color &color) {
    if (!DebugDraw::bDebugMode) return;

    m_points.emplace_back(position, color);
}

const bool DebugDraw::isvalid() {
    return m_shader->isvalid();
}

const bool DebugDraw::hasLines() {
    return !m_lines.empty();
}

void DebugDraw::bind() {
    auto camera = Camera::Main();

    m_shader->bind();

    m_shader->setUniform("projection", camera->getPerspesctive());
    m_shader->setUniform("view", camera->getViewmatrix());
}

void DebugDraw::unbind() {
    m_shader->unbind();
    m_lines.clear();
    m_points.clear();
}

void DebugDraw::Renderer() {
    if (!m_shader->isvalid()) return;

    DebugDraw::bind();
    if (DebugDraw::hasLines() ) {
        DebugDraw::drawLines();
    }
    if (DebugDraw::hasPoints()) {
        DebugDraw::drawPoints();
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

