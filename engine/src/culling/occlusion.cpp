#include "occlusion.h"

#include "../geometry/mesh.h"

#include "../resources/resources.h"

std::unique_ptr<BufferManager> Occlusion::m_bufferManager = nullptr;

Shader* Occlusion::m_shaderQuery = nullptr;

u32 Occlusion::count = 0;

void Occlusion::Initialize() {
    m_bufferManager = std::make_unique<BufferManager>(BufferManager::eSTATIC, true);
    
    Mesh m_mesh("Cube");

    m_mesh.addvertex(Vector3(-1.0f, -1.0f,  1.0f));
    m_mesh.addvertex(Vector3(-1.0f,  1.0f,  1.0f));
    m_mesh.addvertex(Vector3(-1.0f, -1.0f, -1.0f));
    m_mesh.addvertex(Vector3(-1.0f,  1.0f, -1.0f));

    m_mesh.addvertex(Vector3(1.0f, -1.0f,  1.0f));
    m_mesh.addvertex(Vector3(1.0f,  1.0f,  1.0f));
    m_mesh.addvertex(Vector3(1.0f, -1.0f, -1.0f));
    m_mesh.addvertex(Vector3(1.0f,  1.0f, -1.0f));

    m_mesh.addtriangle(1, 2, 0);
    m_mesh.addtriangle(3, 6, 2);
    m_mesh.addtriangle(7, 4, 6);
    m_mesh.addtriangle(5, 0, 4);
    
    m_mesh.addtriangle(6, 0, 2);
    m_mesh.addtriangle(3, 5, 7);
    m_mesh.addtriangle(1, 3, 2);
    m_mesh.addtriangle(3, 7, 6);

    m_mesh.addtriangle(7, 5, 4);
    m_mesh.addtriangle(5, 1, 0);
    m_mesh.addtriangle(6, 4, 0);
    m_mesh.addtriangle(3, 1, 5);

    m_bufferManager->bind(BufferManager::eVAO);

    m_bufferManager->bind(BufferManager::eEBO);
    m_bufferManager->pushElements(m_mesh.getelementcount() * sizeof(u32), m_mesh.getelementdata());

    m_bufferManager->bind(BufferManager::eVBO);
    m_bufferManager->pushBuffer(m_mesh.getverticecount() * sizeof(Vertex3D), m_mesh.getverticedata());

    m_bufferManager->attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, position)));

    m_bufferManager->unbind();

    count = m_mesh.getelementcount();

    m_shaderQuery = Resources::Load<Shader>(Filesystem::getpublic(Filesystem::eShader).string() + "/ShaderQuery");
}

Occlusion::Occlusion() : queryID(-1) {
    GL_CHECK(glGenQueries(1, &queryID));
}

const bool Occlusion::isvisible(const Matrix4x4& vp, const Matrix4x4& model) {
    if (!m_shaderQuery || !m_shaderQuery->isvalid()) return false;

    GRender(GRENDERER3D);

    glDisable(GL_CULL_FACE);
                
                // we don't want the queries to actually render something
                glDepthMask(GL_FALSE);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    m_shaderQuery->bind();
    m_shaderQuery->setUniform("vp", vp);
    m_shaderQuery->setUniform("model", model);

    // glBeginQuery(GL_TIME_ELAPSED, queryID);

    GL_CHECK(glBeginQuery(GL_ANY_SAMPLES_PASSED, queryID));

    m_bufferManager->bind(BufferManager::eVAO);
    m_bufferManager->draw(BufferManager::eTriangle, count, BufferManager::eUint, nullptr);

    GL_CHECK(glEndQuery(GL_ANY_SAMPLES_PASSED));

     glEnable(GL_CULL_FACE);
            
            // turn rendering back on
            glDepthMask(GL_TRUE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            // glUseProgram(shader_program);

    while (!samplesPassed) {
        GL_CHECK(glGetQueryObjectiv(queryID, GL_QUERY_RESULT_AVAILABLE, &samplesPassed));
    }
    samplesPassed = -1;
    while (!samplesPassed)
    GL_CHECK(glGetQueryObjectiv(queryID, GL_QUERY_RESULT, &samplesPassed));

    // glEndQuery(GL_TIME_ELAPSED);

    m_bufferManager->unbind();
    m_shaderQuery->unbind();

    if (samplesPassed < 0) {
        std::cout << "O modelo está sendo renderizado." << std::endl;
        return true;
    }
    std::cout << "O modelo NÃO está sendo renderizado." << std::endl;

    return false;
}
