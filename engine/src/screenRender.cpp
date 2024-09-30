#include "screenRender.hpp"

#include "geometry/vertex2D.h"

#include "scriptManager/scriptManager.hpp"

#include "settings.h"

#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gError.h>

#include "core/assetsManager.hpp"

grr::gVertexArray *ScreenRender::m_vertexArray = nullptr;

grr::gShader *ScreenRender::m_shader = nullptr;

grm::u32 ScreenRender::m_vbo = 0;

void ScreenRender::Initialize() {
    if (!ScriptManager::GetShaders) {
        Console::log(LogInfo::ERROR, "Falied to get script function. [ ScriptManager::GetShaders ]");
        return;
    }

    AssetsManager::AddPostLoader(&ScreenRender::RegisterShader);

    m_shader = ScriptManager::GetShaders()[4];
    if (!m_shader) {
        Console::log(LogInfo::ERROR, "Shader points is not valid.");
        return;
    }

    CreateMesh();
}

void ScreenRender::Shutdown() {
    if (m_vertexArray && !Settings::IsEngineRunning()) {
        grr::gVertexArray::Destroy(m_vertexArray);
        m_vertexArray = nullptr;
    }
}

void ScreenRender::Render(grr::gTexture *texture) {
    if (!texture || !texture->isValid() || !m_shader || !m_shader->isValid()) {
        return;
    }

    grr::gShader::Bind(m_shader);

    grr::gTexture::Bind(texture, 0);
    grr::gShader::SetUniform("u_texture", 0);

    m_vertexArray->bind();
    m_vertexArray->DrawArrays(grr::PrimitiveType_Triangles, 6);

    grr::gTexture::Unbind();
    grr::gShader::Unbind();
}

void ScreenRender::CreateMesh() {
    if (m_vertexArray) {
        return;
    }

    std::vector<Vertex2D> vertices = {
        {{-1.0f,  1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 0.0f}},

        {{-1.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 1.0f}}
    };

    m_vbo = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    if (!m_vbo) {
        return Console::log(LogInfo::ERROR, "[ ScreenRender ] Falied to create vbo.");
    }

    m_vertexArray = grr::gVertexArray::Create();
    if (!m_vertexArray) {
        return Console::log(LogInfo::ERROR, "[ ScreenRender ] Falied to create vao.");
    }

    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex2D), grr::gBufferFlags_Static);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex2D), vertices.data());

    grr::gVertexArray::SetAttrib(0, 2, sizeof(Vertex2D), reinterpret_cast<const void *>(offsetof(Vertex2D, position)));
    grr::gVertexArray::SetAttrib(1, 2, sizeof(Vertex2D), reinterpret_cast<const void *>(offsetof(Vertex2D, uv)));

    m_vertexArray->unbind();
}

bool ScreenRender::RegisterShader(void *ptr, const std::string &name) {
    if (name != "screen") {
        return false;
    }
    grr::gShader::Bind(static_cast<grr::gShader *>(ptr));

    grr::gShader::Register("u_texture");
    grr::gShader::Unbind();
    return true;
}
