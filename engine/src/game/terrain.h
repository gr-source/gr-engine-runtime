#pragma once

#include "../common.h"

#include "../geometry/mesh.h"

#include <thread>

#include <gr-render/gVertexArray.h>

class Terrain {
public:
    static void Initialize();
    
    static void Render();

    static void Destroy();
private:
    static void SetupGPU(const Mesh& mesh);

    static void Process();

    static grr::gVertexArray* m_vertexArray;

    static grr::gShader* m_shader;

    static std::thread m_thread;

    static u32 m_indexVBO;

    static Mesh m_mesh;
};


