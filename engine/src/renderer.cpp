#include "renderer.h"

#include "geometry/mesh.h"

#include "core/resources.h"

#include "script/camera.hpp"

grr::gVertexArray *DebugDraw::m_primitive = nullptr;

std::vector<Line> DebugDraw::m_points;

std::vector<Line> DebugDraw::m_lines;

grr::gShader *DebugDraw::m_shader = nullptr;

bool DebugDraw::bDebugMode = false;

grm::u32 DebugDraw::m_vbo = 0;

void DebugDraw::Initialize() {
    auto frag = File::GetFileContent(File::GetFile("/shader/ShaderPrimitive.frag"));
    auto vert = File::GetFileContent(File::GetFile("/shader/ShaderPrimitive.vert"));

    auto fSource = frag.c_str();
    auto vSource = vert.c_str();

    m_shader = grr::gShader::Create(&fSource, &vSource);

    grr::gShader::Bind(m_shader);
    
    grr::gShader::Register("projection");
    grr::gShader::Register("view");
    
    grr::gShader::Unbind();

    // vertex
    m_vbo = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    assert(m_vbo && "Vbo invalid.");

    m_primitive = grr::gVertexArray::Create();
    assert(m_primitive && "Vao invalid.");

    m_primitive->bind();

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::SetAttrib(0, 3, sizeof(Line), reinterpret_cast<void*>(offsetof(Line, position)));
    grr::gVertexArray::SetAttrib(1, 4, sizeof(Line), reinterpret_cast<void*>(offsetof(Line, color)));

    m_primitive->unbind();
}

void DebugDraw::AddLine(const Vector3& start, const Vector3& end, const Color& color) {
    if (!DebugDraw::bDebugMode) return;
    
    m_lines.emplace_back(start, color);
    m_lines.emplace_back(end, color);
}

void DebugDraw::Renderer() {
    if (!Camera::Current) {
        return;
    }
    auto camera = Camera::Current();
    if (!camera) {
        return;
    }

    grr::gShader::Bind(m_shader);

    grr::gShader::SetUniform("projection", 1, camera->getPerspesctive());
    grr::gShader::SetUniform("view", 1, camera->getViewmatrix());

    if (!m_lines.empty()) {
        DebugDraw::DrawLines();
    }
}

void DebugDraw::DrawLines() {
    m_primitive->bind();

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResizeBuffer(m_lines.size() * sizeof(Line), grr::gBufferFlags_Dynamic);
    grr::gVertexArray::SetBufferUpdate(0, m_lines.size() * sizeof(Line), m_lines.data());

    grr::gVertexArray::DrawArrays(grr::PrimitiveType_Lines, (grm::u32)m_lines.size());

    m_primitive->unbind();

    m_lines.clear();
}




