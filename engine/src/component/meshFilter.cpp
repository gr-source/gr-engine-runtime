#include "meshFilter.h"

#include "../bufferManager.h"

std::unordered_map<std::string, std::unique_ptr<BufferManager>> MeshFilter::m_globalMesh;

MeshFilter::MeshFilter(const std::vector<Mesh> &meshes) : m_bufferManager(nullptr), m_verticeCount(0), m_indiceCount(0) {
    auto it = m_globalMesh.find(meshes[0].getname());
    if (it != m_globalMesh.end()) {
        Log::info() << "Mesh: " << meshes[0].getname() << " added in batchin" << Log::end;
        m_bufferManager = (*it).second.get();
        return;
    }

    std::cout << meshes[0].getname() << std::endl;

    std::vector<Vertex3D> m_vertices;
    std::vector<u32> m_indices;

    for (const auto& mesh : meshes) {
        auto& vertices = mesh.getvertices();
        auto& indices = mesh.getindices();

        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());

        for (const auto& index : indices) {
            m_indices.push_back(index + m_verticeCount);
        }
        m_verticeCount += vertices.size();
    }

    m_boundBox.min = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    m_boundBox.max = Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    
    for (const auto& vertex : m_vertices) {
        // Atualize os valores mínimos e máximos para cada dimensão
        m_boundBox.min.x = std::min(m_boundBox.min.x, vertex.position.x);
        m_boundBox.min.y = std::min(m_boundBox.min.y, vertex.position.y);
        m_boundBox.min.z = std::min(m_boundBox.min.z, vertex.position.z);

        m_boundBox.max.x = std::max(m_boundBox.max.x, vertex.position.x);
        m_boundBox.max.y = std::max(m_boundBox.max.y, vertex.position.y);
        m_boundBox.max.z = std::max(m_boundBox.max.z, vertex.position.z);
    }

    m_indiceCount = m_indices.size();

    m_globalMesh.emplace(meshes[0].getname(), std::make_unique<BufferManager>(BufferManager::eSTATIC, true));
    m_bufferManager = m_globalMesh[meshes[0].getname()].get();

    m_bufferManager->bind(BufferManager::eVAO);

    m_bufferManager->bind(BufferManager::eEBO);
    m_bufferManager->pushElements(m_indiceCount * sizeof(u32), m_indices.data());

    m_bufferManager->bind(BufferManager::eVBO);
    m_bufferManager->pushBuffer(m_verticeCount * sizeof(Vertex3D), m_vertices.data());

    AddAttrib(*m_bufferManager);

    m_bufferManager->unbind();
}

void MeshFilter::AddAttrib(BufferManager& bufferManager) {
    /* posiçao do vertice no espaço mundial */
    bufferManager.attribPointer(0, 3, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, position)));

    /* normal do objeto noe spaço mundial */
    bufferManager.attribPointer(1, 3, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, normal)));

    /* uv usada para desenhar textura */
    bufferManager.attribPointer(2, 2, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, uv)));

    /* tangente do objeto espaço mundial */
    bufferManager.attribPointer(3, 2, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, tangent)));

    /* bitangent usado pra luz espaço mundial */
    bufferManager.attribPointer(4, 2, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, bitangent)));

    /* id do material attribuido a vertice */
    bufferManager.attribPointer(5, 1, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, materialIndex)));
    
    /* id do bone */
    bufferManager.attribPointer(6, 4, GL_INT, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, boneIndex)));

    /* peso do bone attibuido ao vertice */
    bufferManager.attribPointer(7, 4, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, weight)));
}


