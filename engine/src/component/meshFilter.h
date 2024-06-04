#pragma once

#include "../ecs/entity.h"

#include "../geometry/boundBox.h"
#include "../geometry/mesh.h"

#include "../bufferManager.h"

class MeshFilter : public Component {
public:
    MeshFilter(const std::vector<Mesh>& meshes);
    BufferManager* m_bufferManager;

    BoundBox m_boundBox;

    u32 m_verticeCount;
    u32 m_indiceCount;
private:
    static std::unordered_map<std::string, std::unique_ptr<BufferManager>> m_globalMesh;


    void AddAttrib(BufferManager& bufferManager);
};