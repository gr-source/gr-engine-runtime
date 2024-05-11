#pragma once

#include "../Global.h"

struct BoneInfo {
    std::uint32_t id;
    
    char name[256];

    Matrix4x4 offset;

    BoneInfo(std::uint32_t id = -1, const Matrix4x4& offset = Matrix4x4::identityMatrix) : id(id), offset(offset) {}
};

struct AssimpNodeData {
    Matrix4x4 transform;
    std::string name;
    std::vector<AssimpNodeData> children;

    bool operator ==(const AssimpNodeData& other) {
        return this == &other;
    }
};

