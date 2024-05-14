#pragma once

#include "../Global.h"

#include "../ecs/Component.h"

#include "../batchingManager.h"
#include "../bufferManager.h"

#include "../geometry/queryRender.h"

#include "../culling/occlusion.h"

struct DrawCall {
    BufferManager buffers;
    bool IsElements;
    GLuint count;

    BoundBox m_boundBox;

    DrawCall() : count(0), IsElements(false) {}
};

class MeshRender : public Component {
public:
    MeshRender(const Mesh& mesh, const std::unordered_map<u32, Material>& materials);
    MeshRender(const std::vector<Mesh>& meshs, const std::unordered_map<u32, Material>& materials);

    void OnInit() override;

    ~MeshRender();

    std::unordered_map<u32, Material> materials;

    std::unique_ptr<Occlusion> m_occlusion;

    DrawCall m_drawcall;

    u32 verticeCount;
    u32 indiceCount;
protected:

    u32 drawcount;

    Transform* m_transform;

    void AddAttrib(BufferManager& pBufferManager);

    void DropTexture(Texture2D* texture);
};



