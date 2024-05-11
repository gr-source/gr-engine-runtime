#include "fbxScene.h"

FbxScene::FbxScene(const std::string& name) : name(name), root("RootNode") {}

const FbxNode& FbxScene::getroot() const {
    return root;
}

void FbxScene::setname(const std::string& value) {
    name = value;
}

const std::string& FbxScene::getname() const {
    return name;
}

const std::map<u32, AnimationClip> &FbxScene::getanimations() const {
    return m_animations;
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
