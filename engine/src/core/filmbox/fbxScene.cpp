#include "fbxScene.hpp"

FbxScene::FbxScene(const std::string & name) : m_name(name) {}

const std::vector<FbxAnimation> &FbxScene::getAnimations() const {
    return m_animation;
}

const FbxMaterial &FbxScene::getMaterial(int idx) const {
    if (idx >= m_materials.size()) {
        throw std::runtime_error("[ FbxMaterial ] Falied to search material. id=" + std::to_string(idx));
    }
    return m_materials[idx];
}

const std::size_t FbxScene::getMaterialCount() const {
    return m_materials.size();
}

const FbxMesh &FbxScene::getMesh(int idx) const {
    if (idx >= m_meshes.size()) {
        throw std::runtime_error("Falied to search mesh");
    }
    return m_meshes[idx];
}

FbxMesh *FbxScene::findMesh(const std::string &name) {
    auto it = std::find_if(m_meshes.begin(), m_meshes.end(), [&name](const auto &mesh) {
        return name == mesh.getName();
    });
    if (it != m_meshes.end()) {
        return &(*it);
    }
    return nullptr;
}

const std::size_t FbxScene::getMeshCount() const {
    return m_meshes.size();
}

const FbxNode &FbxScene::getRootNode() const {
    return m_rootNode;
}

const FbxNode *FbxScene::findNode(const std::string &name) const {
    return readNode(m_rootNode, name);
}

const std::string &FbxScene::getName() const {
    return m_name;
}

const FbxNode *FbxScene::readNode(const FbxNode &node, const std::string &name) const {
    const FbxNode *found = nullptr;
    if (node.getName() == name) {
        found = &node;
        return found;
    }
    for (const auto &child : node.getChildren()) {
        found = readNode(child, name);
        if (found) {
            break;
        }
    }
    return found;
}
