#pragma once


#include "geometry/batching.hpp"
#include "geometry/material.hpp"

#include "default.hpp"

#include <ecs/component.hpp>

class MeshRender : public Component {
public:
    MeshRender() = delete;

    std::vector<std::shared_ptr<Material>> &getMaterials();

    template <typename ...Args>
    Material *addMaterial(Args &&...args);

    void build();

    Batching *getBatchGroup();

private:
    std::vector<std::shared_ptr<Material>> m_materials;

    Batching *m_batched;

    Transform *m_transform;

    bool SetupShader(Mesh *mesh, grr::gShader *shader);
};

template <typename... Args>
inline Material *MeshRender::addMaterial(Args &&...args) {
    return &(*m_materials.emplace_back(std::make_shared<Material>(std::forward<Args>(args)...)));
}
