#pragma once

#include "../common.h"

#include "../ecs/entity.h"

#include "../batchingManager.h"
#include "../bufferManager.h"

#include "../culling/occlusion.h"

#include "../geometry/material.h"
#include "../geometry/boundBox.h"
#include "../geometry/batchedGeometry.h"

class MeshRender : public Component {
public:
    MeshRender(const std::vector<Mesh> &meshes, const std::unordered_map<u32, Material>& materials);

    void OnInit() override;

    std::unordered_map<u32, Material> m_materials;

    BoundBox m_boundBox;
protected:
    std::vector<Mesh> m_meshes;

    Transform* m_transform;

};



