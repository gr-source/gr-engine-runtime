#include "screenRender.hpp"

#include "geometry/vertex2D.h"

#include "scriptManager/scriptManager.hpp"

#include "settings.h"

grr::gVertexArray *ScreenRender::m_vertexArray = nullptr;

grr::gShader *ScreenRender::m_shader = nullptr;

grm::u32 ScreenRender::m_vbo = 0;

void ScreenRender::Initialize() {
    if (!ScriptManager::GetShader) {
        return;
    }
    auto shaders = ScriptManager::GetShader();
    auto it = shaders.find(1);
    if (it == shaders.end()) {
        return Console::Print(LogInfo::ERROR, "Falied to find postprocess shader 'id=1'");
    }
    auto &&[frag, vert] = (*it).second;

    auto dataF = frag.c_str();
    auto dataV = vert.c_str();

    Console::Print(LogInfo::INFO, "Postprocess loading shader...");

    m_shader = grr::gShader::Create(&dataF, &dataV);
    if (!m_shader || !m_shader->isValid()) {
        std::stringstream iss(grr::gError::GetError());

        std::string line;
        while (std::getline(iss, line)) {
            Console::Print(LogInfo::ERROR, line);
        }
        grr::gError::Clean();
        return;
    }

    m_shader->bind();

    grr::gShader::Register("u_texture");
    m_shader->unbind();

    CreateMesh();
}

void ScreenRender::Shutdown() {
    if (m_shader) {
        delete m_shader;
    }
    m_shader = nullptr;

    if (m_vertexArray && !Settings::IsEngineRunning) {
        m_vertexArray->destroy();
        m_vertexArray = nullptr;
    }
}

void ScreenRender::Render(grr::gTexture *texture) {
    if (!texture || !texture->isValid()) {
        return Console::Print(LogInfo::ERROR, "[ ScreenRender ] Texture do not is valid.");
    }

    if (!m_shader || !m_shader->isValid()) {
        return Console::Print(LogInfo::ERROR, "[ ScreenRender ] Shader do not is valid.");
    }

    m_shader->bind();

    grr::gTexture::Bind(texture, 0);
    grr::gShader::SetUniform("u_texture", 0);

    m_vertexArray->bind();
    m_vertexArray->DrawArrays(grr::PrimitiveType_Triangles, 6);

    grr::gTexture::Unbind();
    m_shader->unbind();
}

void ScreenRender::CreateMesh() {
    if (m_vertexArray) {
        return;
    }

    std::vector<Vertex2D> vertices = {
        {Vector2(-1.0f,  1.0f), Vector2(0.0f, 1.0f)},
        {Vector2(-1.0f, -1.0f), Vector2(0.0f, 0.0f)},
        {Vector2( 1.0f, -1.0f), Vector2(1.0f, 0.0f)},

        {Vector2(-1.0f,  1.0f), Vector2(0.0f, 1.0f)},
        {Vector2( 1.0f, -1.0f), Vector2(1.0f, 0.0f)},
        {Vector2( 1.0f,  1.0f), Vector2(1.0f, 1.0f)}
    };

    m_vbo = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    if (!m_vbo) {
        return Console::Print(LogInfo::ERROR, "[ ScreenRender ] Falied to create vbo.");
    }

    m_vertexArray = grr::gVertexArray::Create();
    if (!m_vertexArray) {
        return Console::Print(LogInfo::ERROR, "[ ScreenRender ] Falied to create vao.");
    }

    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResize(vertices.size() * sizeof(Vertex2D));
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex2D), vertices.data());

    grr::gVertexArray::SetAttrib(0, 2, sizeof(Vertex2D), reinterpret_cast<const void *>(offsetof(Vertex2D, position)));
    grr::gVertexArray::SetAttrib(1, 2, sizeof(Vertex2D), reinterpret_cast<const void *>(offsetof(Vertex2D, uv)));

    m_vertexArray->unbind();
}
