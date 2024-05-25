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

    std::unordered_map<u32, Material> materials;

    std::unique_ptr<Occlusion> m_occlusion;

    // DrawCall m_drawcall;

    BoundBox m_boundBox;

    std::unique_ptr<BufferManager> m_bufferManager;

    u32 verticeCount;
    u32 indiceCount;
protected:

    u32 drawcount;

    Transform* m_transform;

    void AddAttrib(BufferManager& pBufferManager);
};



