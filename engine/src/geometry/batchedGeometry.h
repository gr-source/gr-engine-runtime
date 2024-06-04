#pragma once

#include "../bufferManager.h"
#include "../common.h"

#include "mesh.h"

class BatchedGeometry {
public:
    BatchedGeometry(ulong size);

    void add(GameObject* gameobject);

    void remove(GameObject* gameobject);

    const std::vector<GameObject*>& getObjects() const;

    BufferManager* getBufferManager();

    static std::unordered_map<ulong, std::unique_ptr<BatchedGeometry>> m_batchins;

    Mesh m_mesh;

private:
    std::unique_ptr<BufferManager> m_bufferManager;

    std::vector<GameObject*> m_objects;
};

