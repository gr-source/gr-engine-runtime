#include "screenRender.hpp"

#include <core/geometry/vertex2D.hpp>

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
    if (!m_shader) {
        AssetsManager::LoadShader(
            "screen",
            File::GetAsset("/shader/screen.frag"),
            File::GetAsset("/shader/screen.vert"),
            &ScreenRender::RegisterShader
        );
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


    grr::gTexture::Bind(texture, 0);
    grr::gShader::SetUniform("u_texture", 0);

    grr::gShader::Bind(m_shader);
    grr::gShader::Flush();
    
    m_vertexArray->bind();
    
    grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, 6);

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

    m_vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
    if (!m_vbo) {
        return Console::log(Log::ERROR, "[ ScreenRender ] Falied to create vbo.");
    }

    m_vertexArray = grr::gVertexArray::Create();
    if (!m_vertexArray) {
        return Console::log(Log::ERROR, "[ ScreenRender ] Falied to create vao.");
    }

    m_vertexArray->bind();

    grr::gVertexArray::Bind(m_vbo);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex2D), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex2D), vertices.data());

    grr::gVertexArray::SetAttrib(0, 2, sizeof(Vertex2D), reinterpret_cast<const void *>(offsetof(Vertex2D, position)));
    grr::gVertexArray::SetAttrib(1, 2, sizeof(Vertex2D), reinterpret_cast<const void *>(offsetof(Vertex2D, uv)));

    m_vertexArray->unbind();
}

void ScreenRender::RegisterShader(grr::gShader *shader) {
    grr::gShader::Register("u_texture", UniformType::INT, 1, 0);

    m_shader = shader;
}
