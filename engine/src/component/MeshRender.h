#pragma once

#include "../common.h"

#include "../ecs/entity.h"

#include "../bufferManager.h"

#include "../culling/occlusion.h"

#include "../geometry/material.h"
#include "../geometry/boundBox.h"
#include "../geometry/batchedGeometry.h"

class MeshRender : public Component {
public:
    MeshRender();
    MeshRender(const Mesh &mesh);
    ~MeshRender();

    const BoundBox& getBoundBox() const;

    Mesh& getMesh();

    void buildMesh();

    void draw();

    std::unordered_map<u32, Material> m_materials;

private:
    BoundBox m_boundBox;

    Mesh m_outmesh;

    u32 m_indexEBO;

    u32 m_indexVBO;

    grr::gVertexArray* m_vertexArray;
};



