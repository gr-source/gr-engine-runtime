#include "fbxScene.hpp"

FbxScene::FbxScene(const std::string & name) : m_name(name) {}

FbxMaterial &FbxScene::getMaterial(int idx) {
    return m_materials[idx];
}

const std::size_t FbxScene::getMaterialCount() const {
    return m_materials.size();
}

FbxMesh &FbxScene::getMesh(int idx) {
    return m_meshes[idx];
}

const std::size_t FbxScene::getMeshCount() const {
    return m_meshes.size();
}

const FbxNode &FbxScene::getRootNode() const {
    return m_rootNode;
}

const std::string &FbxScene::getName() const {
    return m_name;
}
