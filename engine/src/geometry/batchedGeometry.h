#pragma once

#include "../bufferManager.h"
#include "../common.h"

#include "mesh.h"

class BatchedGeometry {
public:
    BatchedGeometry(ulong size);

    template <typename... Args>
    void addMesh(Args&&... args);

private:
    std::unique_ptr<BufferManager> m_bufferManager;
};

template <typename... Args>
inline void BatchedGeometry::addMesh(Args &&...args) {
}
