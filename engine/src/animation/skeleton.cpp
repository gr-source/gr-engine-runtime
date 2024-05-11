#include "skeleton.h"

Skeleton::Skeleton(const Skeleton& rhs) : m_root(rhs.m_root), m_count(0) {}

Bone* Skeleton::findBone(const std::string& name) {
    return nullptr;
}

Bone& Skeleton::getroot() {
    return m_root;
}

bool Skeleton::hasbone() const {
    return !m_root.m_children.empty();
}

u32 Skeleton::getcount() const {
    return 0;
}
