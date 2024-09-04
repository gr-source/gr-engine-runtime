#pragma once

#include "fbxScene.hpp"

#include <assimp/cimport.h>

#include "../thread.hpp"

#include <assimp/scene.h>

class Filmbox {
public:
    Filmbox(const std::string &path);
    ~Filmbox();

    void setPostProcess(std::function<void(const FbxScene *, Filmbox *)> func);

    void load();

    const bool isValid() const;
    
private:
    void doInBackground(const std::string &path);

    void sceneLoader(const aiScene *scene);

    void loadAnimation(const aiScene *scene);

    void addKeys(aiNodeAnim *mNodeAnim, FbxChannel &channel);

    void loadMaterial(const aiScene *scene);

    void processTexture(aiMaterial *iMaterial, FbxMaterial& material);

    void loadMesh(const aiScene *scene);

    void processMesh(aiMesh *mMesh);

    void inserMesh(FbxMesh *mesh, aiMesh *mMesh);

    void processNode(aiNode *mNode, FbxNode& node, const aiScene *scene);

    std::function<void(const FbxScene *, Filmbox *)> m_funcPost;

    std::unique_ptr<FbxScene> m_fbxScene;

    std::unique_ptr<Thread> m_thread;

    std::string m_path;

    std::unordered_map<std::string, int> m_indexMaterial;
};


