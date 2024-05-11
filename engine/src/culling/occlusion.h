#pragma once

#include "../bufferManager.h"

class Occlusion {
public:
    Occlusion();

    static void Initialize();

    const bool isvisible(const Matrix4x4& vp, const Matrix4x4& model);
    u32 queryID;
protected:
    static std::unique_ptr<BufferManager> m_bufferManager;

    static Shader* m_shaderQuery;


    u32 count;
};