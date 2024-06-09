#pragma once

#include "../bufferManager.h"
#include "../common.h"

#include "../culling/frustum.h"

#include "material.h"
#include "mesh.h"

#include <gr-render/gVertexArray.h>

class BatchedGeometry {
public:
    BatchedGeometry(const Mesh& mesh);
    ~BatchedGeometry();

    static std::unordered_map<ulong, std::unique_ptr<BatchedGeometry>> m_batchins;

    void add(GameObject* gameobject);

    void remove(GameObject* gameobject);

    void render(const Frustum& frustum, Shader* shader);

    // void calculate();

    const std::vector<GameObject*>& getObjects() const;

    std::unordered_map<u32, Material> m_materials;

    Mesh m_mesh;

    // std::vector<Vertex3D> m_vertices;
    // std::vector<u32> m_indices;

    u32 vertexCount;

    std::vector<Matrix4x4> models;


private:
    grr::gVertexArray* m_vertexArray;

    u32 m_indexVBO;
    u32 m_indexEBO;
    u32 m_indexPrim;

    // std::unique_ptr<BufferManager> m_bufferManager;

    std::vector<GameObject*> m_objects;

    bool m_isCalculated;
};

