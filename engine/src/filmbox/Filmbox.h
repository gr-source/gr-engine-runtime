#pragma once

#include "../Global.h"

#include "../geometry/mesh.h"
#include "../animation/bone.h"
#include "../animation/animation.h"
#include "../animation/animationClip.h"

#include "BoneInfo.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "fbxScene.h"

class Filmbox {
public:
    enum : u8 {
        eNoLoadTexture = 1 << 1,
    };

    Filmbox(const std::string& path);
    ~Filmbox();

    void load();
    
    FbxScene& getscene();

    void SetChangeGameObject(std::function<void(const FbxNode&, const FbxNode*, FbxScene&)> func);

    void SetLoadingStatus(std::function<void(u32, u32, const std::string&)> func);

    void SetLoaded(const std::function<void(const FbxScene&)> func);

protected:

    void setupLoader();

    void loadCamera(const aiScene* scene);

    void loadLight(const aiScene* scene);

    void loadAnimationKeys(Keyframe& keyFrame, const aiNodeAnim* nodeAnim);

    void loadAnimationKeyPositions(Keyframe& keyFrame, const aiNodeAnim* nodeAnim);

    void loadAnimationKeyRotations(Keyframe& keyFrame, const aiNodeAnim* nodeAnim);

    void loadAnimationKeyScales(Keyframe& keyFrame, const aiNodeAnim* nodeAnim);

    void loadAnimations(const aiScene* scene);

    void setupVertice(aiMesh* nodeMesh, Mesh& mesh);

    void setupFace(aiMesh* nodeMesh, Mesh& mesh);

    void setupSkeleton(aiNode* node, Bone* bone);

    void setupBone(aiMesh* nodeMesh, Mesh& mesh, const aiScene* scene);

    void loadvertices(aiMesh* pMesh, Mesh& mesh, const aiScene* scene);

    void loadMeshes(const aiScene* scene);

    void loadMaterials(const aiScene* scene);

    void loadMaterialTexture(aiMaterial* pMaterial, Material& material);

    void setupNode(aiNode* node, const aiScene* scene, FbxNode& fbxNode);

    void loadNode(aiNode* node, const aiScene* scene, FbxNode& fbxNode);

    FbxScene fbxScene;

    u32 m_BoneCounter;

    std::unordered_map<std::string, Bone> m_boneMap;
    std::unordered_map<std::string, bool> m_isBone;

    std::function<void(const FbxNode&, const FbxNode*, FbxScene&)> m_changenewgameobject;

    std::function<void(u32, u32, const std::string&)> m_changeloading;

    std::function<void(const FbxScene&)> m_loaded;

    Assimp::Importer importer;

    std::string path;

    std::thread m_thread;

    std::mutex mt;
};


