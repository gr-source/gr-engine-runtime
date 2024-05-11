#include "fbxNode.h"

FbxNode::FbxNode(const std::string& name, FbxNode* parent, u8 type) : name(name), parent(parent), m_type(type) {}

bool FbxNode::haschild() const {
    return !children.empty();
}

bool FbxNode::hasmesh() const {
    return !meshs.empty();
}

const std::vector<Mesh>& FbxNode::getmeshs() const {
    return meshs;
}

void FbxNode::setname(const std::string& value) {
    name = value;
}

const std::string& FbxNode::getname() const {
    return name;
}

const u8 &FbxNode::gettype() const {
    return m_type;
}

const std::vector<FbxNode> &FbxNode::getchildren() const {
    return children;
}
