#pragma once

#include "../common.h"

#include "../ecs/entity.h"

#include "../batchingManager.h"
#include "../bufferManager.h"

#include "../culling/occlusion.h"

#include "../geometry/boundBox.h"

class MeshRender : public Component {
public:
    MeshRender(const std::vector<Mesh> &meshes, const std::unordered_map<u32, Material>& materials);

    void OnInit() override;

    ~MeshRender();

    std::unordered_map<u32, Material> m_materials;

protected:
    Transform* m_transform;

    static std::unordered_map<std::size_t, bool> m_batchins;
};



