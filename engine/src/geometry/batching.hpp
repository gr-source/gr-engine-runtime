#pragma once

#include <gr-render/gRender.h>

#include "default.hpp"

#include "material.h"

class Batching {
public:
    Batching(std::unordered_map<int, Material> &materials);

    void insert(Transform *transform, Mesh *mesh);

    void draw();

    void rebuild();

private:
    std::unordered_map<int, Material> m_materials;

    grr::gVertexArray *m_vertexArray;

    std::vector<std::tuple<Transform *, Mesh *>> m_meshes;

    grm::u32 m_count;

    grm::u32 m_VBO;

    grm::u32 m_EBO;

    bool bHasIndice;


    void compile(Mesh *mesh);

    void setupMaterial();
};




