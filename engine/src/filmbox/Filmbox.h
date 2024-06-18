#pragma once

#include "../Global.h"

#include "../geometry/mesh.h"
#include "../animation/bone.h"
#include "../animation/animation.h"
#include "../animation/animationClip.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "fbxScene.h"

class Filmbox {
public:
    static Filmbox* Load(const std::string& path);

    Filmbox();

    const FbxScene* getScene();

    void destroy();
private:
    std::string m_path;

    std::unique_ptr<FbxScene> m_fbxScene;

    void process();

    void StartupScene(const aiScene* scene);

    void ProcessNodes(const aiScene* scene);

    void ProcessChild(aiNode* ainode, FbxNode* fbxNode);

    void SetupNode(aiNode* ainode, FbxNode* fbxNode);

    void LoadMeshes(const aiScene* scene);

    void SetupBone(aiMesh* aimesh);

    void SetupSkeleton(aiNode* node, Bone*& bone);

    void SetupVertice(aiMesh *aimesh, FbxMesh &mesh);

    void LoadMaterials(const aiScene* scene);

    void SetupTexture(aiMaterial* aimaterial, Material& material);

    void TextureLoadSucessfull(Texture* texture);

    void LoadTextures(const aiScene* scene);

    void LoadAnimations(const aiScene* scene);

    void LoadKeyframeScale(aiNodeAnim* nodeAnim, Keyframe& key);

    void LoadKeyframeRotation(aiNodeAnim* nodeAnim, Keyframe& key);

    void LoadKeyframePosition(aiNodeAnim* nodeAnim, Keyframe& key);
    
    /*void load();
    
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

    void setupSkeleton(aiNode* node, Bone*& bone);

    void setupBone(FbxNode& fbxNode, Mesh& mesh, aiMesh* nodeMesh, const aiScene* scene);

    void addBoneData(Vertex3D& vertex, u32 index, float weight);

    void loadvertices(FbxNode& fbxNode, Mesh& mesh, aiMesh* pMesh, const aiScene* scene);

    void loadMeshes(const aiScene* scene);

    void loadMaterials(const aiScene* scene);

    void loadMaterialTexture(aiMaterial* pMaterial, Material& material);

    void setupBoneInNode(FbxNode& node, aiMesh* mesh, const aiScene* scene);

    void setupNode(aiNode* node, const aiScene* scene, FbxNode& fbxNode);

    void loadNode(aiNode* node, const aiScene* scene, FbxNode& fbxNode);

    void TextureLoadSucessfull(Texture* texture);

    FbxScene fbxScene;

    u32 m_BoneCounter;

    std::function<void(const FbxNode&, const FbxNode*, FbxScene&)> m_changenewgameobject;

    std::function<void(u32, u32, const std::string&)> m_changeloading;

    std::function<void(const FbxScene&)> m_loaded;

    Assimp::Importer importer;

    std::string path;

    std::thread m_thread;

    std::mutex mt;
    */
};
