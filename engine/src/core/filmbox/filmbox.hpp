#pragma once

#include "fbxScene.hpp"

#include <assimp/cimport.h>
#include <assimp/scene.h>

#include "../thread.hpp"

class Filmbox {
public:
    Filmbox(const std::string &path);
    ~Filmbox();

    void setPostProcess(std::function<void(const FbxScene *)> func);

    void load();
private:
    void doInBackground(const std::string &path);

    void sceneLoader(const aiScene *scene);

    void loadAnimation(const aiScene *scene);

    void addKeys(aiNodeAnim *mNodeAnim, FbxChannel &channel);

    void loadMaterial(const aiScene *scene);

    void processTexture(aiMaterial *iMaterial, FbxMaterial& material);

    void loadMesh(const aiScene *scene);

    void processMesh(aiMesh *mMesh);

    void processNode(aiNode *mNode, FbxNode& node);

    std::function<void(const FbxScene *)> m_funcPost;

    std::unique_ptr<FbxScene> m_fbxScene;

    std::unique_ptr<Thread> m_thread;

    std::string m_path;
};


