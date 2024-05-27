#pragma once

#include "../Global.h"

#include "../ecs/entity.h"

#include "../batchingManager.h"
#include "../bufferManager.h"

#include "../geometry/queryRender.h"

#include "../culling/occlusion.h"

class MeshRender : public Component {
public:
    MeshRender(const std::vector<Mesh>& meshs, const std::unordered_map<u32, Material>& materials);

    void OnInit() override;

    ~MeshRender();

    std::unique_ptr<BufferManager> m_bufferManager;

    std::unordered_map<u32, Material> materials;

    BoundBox m_boundBox;

    u32 verticeCount;
    u32 indiceCount;
protected:

    u32 drawcount;

    Transform* m_transform;

    void AddAttrib(BufferManager& pBufferManager);
};



