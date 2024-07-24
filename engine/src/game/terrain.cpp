#include "terrain.h"

#include "../eventManager.h"

grr::gVertexArray* Terrain::m_vertexArray = nullptr;

grr::gShader* Terrain::m_shader = nullptr;

std::thread Terrain::m_thread;

u32 Terrain::m_indexVBO = 0;

Mesh Terrain::m_mesh;

const char* fragment = R"(
    #version 330 core

    out vec4 FragColor;

    void main() {
        FragColor = vec4(1.0);
    }
)";

const char* vertex = R"(
    #version 330 core

    layout (location = 0) in vec3 in_pos;
)";

void Terrain::Initialize() {
    m_vertexArray = grr::gVertexArray::Create();
    m_vertexArray->bind();
    
    m_indexVBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO);
    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), ((void*)offsetof(Vertex3D, position)));

    m_vertexArray->unbind();

    m_thread = std::thread(Terrain::Process);
}

void Terrain::Destroy() {
    if (m_thread.joinable()) {
        m_thread.detach();
    }
}

void Terrain::SetupGPU(const Mesh &mesh) {
    m_vertexArray->bind();
    
    grr::gVertexArray::Bind(m_indexVBO);
    grr::gVertexArray::SetBufferUpdate(0, mesh.getverticecount() * sizeof(Vertex3D), mesh.getverticedata());

    m_vertexArray->unbind();
}

void Terrain::Process() {
    u32 size = 32;

    for (u32 x=0;x<size;x++) {
        for (u32 z=0;z<size;z++) {
            m_mesh.addvertex(Vector3(x * 1.0f, 0.0f, z * 1.0f));
        }
    }

    EventManager::addEvent(Terrain::SetupGPU, m_mesh);
}
