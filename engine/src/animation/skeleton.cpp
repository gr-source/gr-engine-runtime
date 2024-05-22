#include "skeleton.h"

Skeleton::Skeleton(const Skeleton& rhs) : m_root(rhs.m_root) {}

Bone* Skeleton::findBone(const std::string& name) {
    return nullptr;
}

Bone& Skeleton::getroot() {
    return m_root;
}

const bool Skeleton::hasbone() const {
    return m_root.getname() != "none";
}


