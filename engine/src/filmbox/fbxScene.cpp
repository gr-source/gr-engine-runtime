#include "fbxScene.h"

FbxScene::FbxScene(const std::string& name) : name(name), m_rootNode(nullptr) {}

const FbxNode* FbxScene::getRoot() const {
    return m_rootNode.get();
}

void FbxScene::setname(const std::string& value) {
    name = value;
}

const std::string& FbxScene::getname() const {
    return name;
}

const std::vector<AnimationClip> &FbxScene::getanimations() const {
    return m_animations;
}

Skeleton *FbxScene::findSkeleton(const std::string &name) {
    auto it = std::find_if(m_skeletons.begin(), m_skeletons.end(), [&](auto& skeleton) {
        return skeleton.getname() == name;
    });
    if (it != m_skeletons.end()) {
        return &(*it);
    }
    return nullptr;
}

const FbxLight *FbxScene::findlight(const std::string &name) const {
    auto it = m_lights.find(name);
    if (it != m_lights.end()) {
        return &(*it).second;
    }
    return nullptr;
}

const FbxCamera *FbxScene::findcamera(const std::string &name) const {
    auto it = m_cameras.find(name);
    if (it != m_cameras.end()) {
        return &(*it).second;
    }
    return nullptr;
}

Material *FbxScene::findMaterial(u32 index) {
    auto material = m_materials.find(index);
    if (material != m_materials.end()) {
        return &(*material).second;
    }
    return nullptr;
}

const std::unordered_map<u32, Material> &FbxScene::getmaterials() const {
    return m_materials;
}

Mesh *FbxScene::findmesh(u32 index) {
    auto it = m_meshes.find(index);
    if (it != m_meshes.end()) {
        return &(*it).second;
    }
    return nullptr;
}
