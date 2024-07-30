#pragma once

#include "fbxScene.hpp"

#include <assimp/cimport.h>

#include "../thread.hpp"

#include <assimp/scene.h>

class Filmbox {
public:
    Filmbox(const std::string &path);

    void setPostProcess(std::function<void(const FbxScene *)> func);

private:
    void doInBackground(const std::string &path);

    void sceneLoader(const aiScene *scene);

    void loadMaterial(const aiScene *scene);

    void loadMesh(const aiScene *scene);

    void processMesh(aiMesh *mMesh);

    void processNode(aiNode *mNode, FbxNode& node);

    std::unique_ptr<FbxScene> m_fbxScene;

    std::unique_ptr<Thread> m_thread;

    std::function<void(const FbxScene *)> m_funcPost;
};


