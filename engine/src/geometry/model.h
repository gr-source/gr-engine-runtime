#pragma once

#include "material.h"
#include "mesh.h"

#include "../common.h"

class Model {
public:
    Model(const std::string& name = "unnamed");

private:
    std::unordered_map<grm::u32, Material> m_materials;

    std::string m_name;

    Mesh m_mesh;

};
