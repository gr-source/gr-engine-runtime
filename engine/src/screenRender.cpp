#include "screenRender.hpp"

#include "geometry/vertex2D.h"

#include "scriptManager/scriptManager.hpp"

#include "settings.h"

#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gError.h>

grr::gVertexArray *ScreenRender::m_vertexArray = nullptr;

grr::gShader *ScreenRender::m_shader = nullptr;

grm::u32 ScreenRender::m_vbo = 0;

void ScreenRender::Initialize() {
    if (!ScriptManager::GetShader) {
        Console::log(LogInfo::ERROR, "Falied to get script function. [ ScriptManager::GetShader ]");
        return;
    }
    
    auto shaders = ScriptManager::GetShader();
    auto it = shaders.find(1);
    if (it == shaders.end()) {
        return Console::log(LogInfo::ERROR, "Falied to find postprocess shader 'id=1'");
    }
    auto &&[frag, vert] = (*it).second;

    auto dataF = frag.c_str();
    auto dataV = vert.c_str();

    Console::log(LogInfo::INFO, "Postprocess loading shader...");

    m_shader = grr::gShader::Create(&dataF, &dataV);
    if (!m_shader || !m_shader->isValid()) {
        std::stringstream iss(grr::gError::GetError());

        std::string line;
        while (std::getline(iss, line)) {
            Console::log(LogInfo::ERROR, line);
        }
        return;
    }

    grr::gShader::Bind(m_shader);

    grr::gShader::Register("u_texture");
    grr::gShader::Unbind();

    CreateMesh();
}

void ScreenRender::Shutdown() {
    if (m_shader) {
        delete m_shader;
    }
    m_shader = nullptr;

    if (m_vertexArray && !Settings::IsEngineRunning()) {
        m_vertexArray->destroy();
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







