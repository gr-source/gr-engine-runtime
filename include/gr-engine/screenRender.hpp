#pragma once

#include <gr-render/gRender.h>

#include <gr-base/script/default.hpp>

class ScreenRender {
public:
    static void Initialize();

    static void Shutdown();

    static void Render(Camera *camera);

private:
    static grr::gVertexArray *m_vertexArray;

    static grr::gShader *m_shader;

    static u32 m_vbo;

    static void CreateMesh();
};





