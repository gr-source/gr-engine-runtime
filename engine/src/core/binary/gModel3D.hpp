#pragma once

#include <string>
#include <memory>

#include "core/filmbox/default.hpp"

class gModel3D {
public:
    gModel3D() = delete;
    ~gModel3D() = delete;

    static void ImportFBX(const std::string &path);

private:
    static void SetupScene(std::shared_ptr<FbxScene> scene);

    static void WriteHeader(const FbxScene *scene, const FbxNode *node);

    static void WriteNode(const FbxScene *scene, const FbxNode *node);
};



