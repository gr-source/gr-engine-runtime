#pragma once

#include <gr-render/gRender.h>

class ScreenRender {
public:
    static void Initialize();

    static void Shutdown();

    static void Render(grr::gTexture *texture, const Rect &rect);

private:
    static grr::gVertexArray *m_vertexArray;

    static grr::gShader *m_shader;

    static grm::u32 m_vbo;

    static void CreateMesh();

    static bool RegisterShader(grr::gShader *shader);
};





