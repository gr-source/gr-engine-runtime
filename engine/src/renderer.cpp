#include "renderer.h"

#include <gr-base/core/geometry/mesh.hpp>

#include <gr-base/core/assetsManager.hpp>

#include <gr-base/script/camera.hpp>

#include <gr-render/gVertexArray.h>
#include <gr-render/gShader.h>

grr::gVertexArray *DebugDraw::m_primitive = nullptr;

std::vector<Line> DebugDraw::m_points;

std::vector<Line> DebugDraw::m_lines;

grr::gShader *DebugDraw::m_shader = nullptr;

bool DebugDraw::bDebugMode = false;

grm::u32 DebugDraw::m_vbo = 0;

void DebugDraw::Initialize() {
    /*
    AssetsManager::LoadShader(
        "debug_mode",
        File::GetFile("/assets/shader/primitive/line.frag", true),
        File::GetFile("/assets/shader/primitive/line.vert", true),
        &DebugDraw::SetupShader);
    */
}

void DebugDraw::Shutdown() {
    if (m_primitive) {
        grr::gVertexArray::Destroy(m_primitive);
        m_primitive = nullptr;
    }

    if (m_vbo) {
        grr::gVertexArray::DeleteBuffer(m_vbo);
        m_vbo = 0;
    }
}

void DebugDraw::AddLine(const Vector3& start, const Vector3& end, const Color& color) {
    m_lines.emplace_back(start, color);
    m_lines.emplace_back(end, color);
}

void DebugDraw::Renderer(const Camera *camera) {
    /*
    if (!camera || !m_shader || !m_shader->isValid()) {
        return;
    }

    grr::gShader::SetUniform("u_vp", camera->getPerspesctive() * camera->getViewmatrix());

    if (!m_lines.empty()) {
        DebugDraw::DrawLines();
    }
    */
}

bool DebugDraw::SetupShader(grr::gShader *shader) {
    /*
    if (!shader) { return true; }

    grr::gShader::Register("u_vp", UniformType::MAT4, 1, Matrix4x4::identityMatrix);

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

    m_shader = shader;

    */
    return true;
}

void DebugDraw::DrawLines() {
    /*
    m_primitive->bind();

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResizeBuffer(m_lines.size() * sizeof(Line), BufferUsage::DYNAMIC);
    grr::gVertexArray::SetBufferUpdate(0, m_lines.size() * sizeof(Line), m_lines.data());

    grr::gShader::Flush();

    grr::gVertexArray::DrawArrays(PrimitiveType::LINES, (grm::u32)m_lines.size());

    m_primitive->unbind();

    m_lines.clear();
    */
}
