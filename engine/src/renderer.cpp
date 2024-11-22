#include "renderer.h"

#include "geometry/mesh.hpp"

#include "core/assetsManager.hpp"

#include "script/camera.hpp"

#include <gr-render/gVertexArray.h>
#include <gr-render/gShader.h>

grr::gVertexArray *DebugDraw::m_primitive = nullptr;

std::vector<Line> DebugDraw::m_points;

std::vector<Line> DebugDraw::m_lines;

grr::gShader *DebugDraw::m_shader = nullptr;

bool DebugDraw::bDebugMode = false;

grm::u32 DebugDraw::m_vbo = 0;

void DebugDraw::Initialize() {
    auto frag = File::GetFileContent(File::GetFile("/shader/ShaderPrimitive.frag", true));
    auto vert = File::GetFileContent(File::GetFile("/shader/ShaderPrimitive.vert", true));
    if (frag.empty() || vert.empty()) {
        throw std::runtime_error("Frag or Vert shader is empty: " + File::GetFile("/shader/ShaderPrimitive.frag") + " - " + File::GetFile("/shader/ShaderPrimitive.vert"));
    }

    auto fSource = frag.c_str();
    auto vSource = vert.c_str();

    m_shader = grr::gShader::Create(&fSource, &vSource);
    if (!m_shader || !m_shader->isValid()) {
        throw std::runtime_error("Shader do not is valid: " + frag + " - " + vert);
    }
    grr::gShader::Bind(m_shader);
    
    grr::gShader::Register("projection", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    grr::gShader::Register("view", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    
    grr::gShader::Unbind();

    // vertex
    m_vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
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
    /*
    if (!Camera::Current || !m_shader || !m_shader->isValid()) {
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
    */
}

void DebugDraw::DrawLines() {
    m_primitive->bind();

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResizeBuffer(m_lines.size() * sizeof(Line), BufferUsage::DYNAMIC);
    grr::gVertexArray::SetBufferUpdate(0, m_lines.size() * sizeof(Line), m_lines.data());

    grr::gShader::Flush();
    grr::gVertexArray::DrawArrays(PrimitiveType::LINES, (grm::u32)m_lines.size());

    m_primitive->unbind();

    m_lines.clear();
}




