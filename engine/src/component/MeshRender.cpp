#include "MeshRender.h"

#include "Transform.h"

#include "../bufferManager.h"

#include "../batchingManager.h"

MeshRender::MeshRender(const std::vector<Mesh>& meshes, const std::unordered_map<u32, Material>& materials) : Component(true), materials(materials), verticeCount(0), indiceCount(0), drawcount(0), m_bufferManager(std::make_unique<BufferManager>(BufferManager::eSTATIC, true)) {
    // auto it = BatchingManager::m_batchingRender.find(meshes[0].getname());
    // if (it != BatchingManager::m_batchingRender.end()) {
    //     return;
    // }
    
    std::vector<Vertex3D> m_vertices;
    std::vector<GLuint> m_indices;

    for (const auto& mesh : meshes) {
        auto& vertices = mesh.getvertices();
        auto& indices = mesh.getindices();

        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());

        for (const u32& index : indices) {
            m_indices.push_back(index + verticeCount);
        }
        verticeCount += vertices.size();
    }

    Vector3 minBound = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vector3 maxBound = Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    
    for (const auto& vertex : m_vertices) {
        // Atualize os valores mínimos e máximos para cada dimensão
        minBound.x = std::min(minBound.x, vertex.position.x);
        minBound.y = std::min(minBound.y, vertex.position.y);
        minBound.z = std::min(minBound.z, vertex.position.z);

        maxBound.x = std::max(maxBound.x, vertex.position.x);
        maxBound.y = std::max(maxBound.y, vertex.position.y);
        maxBound.z = std::max(maxBound.z, vertex.position.z);
    }

    m_boundBox.min = minBound;
    m_boundBox.max = maxBound;

    indiceCount = m_indices.size();

    // Configura os buffers
    m_bufferManager->bind(BufferManager::eVAO);

    m_bufferManager->bind(BufferManager::eEBO);
    m_bufferManager->pushElements(indiceCount * sizeof(GLuint), m_indices.data());

    m_bufferManager->bind(BufferManager::eVBO);
    m_bufferManager->pushBuffer(verticeCount * sizeof(Vertex3D), m_vertices.data());

    /* configure o buffer do shader */
    AddAttrib(*m_bufferManager);

    m_bufferManager->unbind();

    gameStatics.vertices += verticeCount;
    gameStatics.indices += indiceCount;

    // BatchingManager::m_batchingRender.emplace(meshes[0].getname(), Batching{m_bufferManager.get()});

    BatchingManager::drawCount += 1;
}

void MeshRender::OnInit() {
    m_transform = &entity->getComponent<Transform>();
}

void MeshRender::AddAttrib(BufferManager& pBufferManager) {
    /* posiçao do vertice no espaço mundial */
    pBufferManager.attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, position)));

    /* normal do objeto noe spaço mundial */
    pBufferManager.attribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, normal)));

    /* uv usada para desenhar textura */
    pBufferManager.attribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, uv)));

    /* tangente do objeto espaço mundial */
    pBufferManager.attribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, tangent)));

    /* bitangent usado pra luz espaço mundial */
    pBufferManager.attribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, bitangent)));

    /* id do material attribuido a vertice */
    pBufferManager.attribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, materialIndex)));
    
    /* id do bone */
    pBufferManager.attribPointer(6, 4, GL_INT, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, boneIndex)));

    /* peso do bone attibuido ao vertice */
    pBufferManager.attribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, weight)));
}

MeshRender::~MeshRender() {
    gameStatics.vertices -= verticeCount;
    gameStatics.indices  -= indiceCount;

    BatchingManager::drawCount -= 1;
}


