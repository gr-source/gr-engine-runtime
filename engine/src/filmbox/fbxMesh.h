#pragma once

#include <string>

class FbxMesh {
public:
    std::string name;

    FbxMesh() = default;

    FbxMesh(const std::string& name);
};
