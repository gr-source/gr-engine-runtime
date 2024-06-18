#include "fbxNode.h"

FbxNode::FbxNode(const std::string& name, FbxNode* parent, u8 type) : name(name), parent(parent), m_type(type), m_skeleton(nullptr) {}

bool FbxNode::haschild() const {
    return !m_children.empty();
}

bool FbxNode::hasmesh() const {
    return !m_meshs.empty();
}

const std::vector<FbxMesh>& FbxNode::getmeshs() const {
    return m_meshs;
}

void FbxNode::setname(const std::string& value) {
    name = value;
}

const std::string& FbxNode::getname() const {
    return name;
}

const Matrix4x4 FbxNode::getGlobalMatrix() const {
    return Math::translate<Matrix4x4>(position) * Math::rotate<Matrix4x4>(rotation) * Math::scale<Matrix4x4>(scale);
}

const u8 &FbxNode::gettype() const {
    return m_type;
}

const std::vector<FbxNode> &FbxNode::getchildren() const {
    return m_children;
}

const bool FbxNode::hasSkeleton() const {
    return m_skeleton != nullptr;
}

const Skeleton *FbxNode::getSkeleton() const {
    return m_skeleton;
}

Material *FbxNode::findMaterial(u32 index) {
    auto it = m_materials.find(index);
    if (it != m_materials.end()) {
        return &(*it).second;
    }
    return nullptr;
}

const std::unordered_map<u32, Material> &FbxNode::getMaterials() const {
    return m_materials;
}

const u32 FbxNode::getMaterialCount() const {
    return static_cast<u32>(m_materials.size());
}
