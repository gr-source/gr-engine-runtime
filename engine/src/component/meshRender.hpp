#pragma once

#include "../ecs/entity.h"

#include "../geometry/material.h"

#include <gr-render/gVertexArray.h>
#include <gr-render/gRender.h>

class MeshRender : public Component {
public:
    std::unordered_map<int, Material> &getMaterials();

    void draw();
private:
    std::unordered_map<int, Material> m_materials;

    grr::gVertexArray *m_vertexArray;

    grr::u32 m_idxVBO;

    grr::u32 m_idxEBO;

    grr::u32 m_count;

    bool isIndice;
};



