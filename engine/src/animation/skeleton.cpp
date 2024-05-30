#include "skeleton.h"

Skeleton::Skeleton(const std::string &name) : m_name(name), m_root(nullptr) {}

Skeleton::~Skeleton() {
    // if (m_root != nullptr) {
    //     delete m_root;
    // }
}

const std::string &Skeleton::getname() const {
    return m_name;
}


Bone *Skeleton::findBone(const std::string &name) {
    return find(m_root, name);
}

Bone* Skeleton::getroot() {
    return m_root;
}

const Bone* Skeleton::getroot() const {
    return m_root;
}

const bool Skeleton::hasbone() const {
    return m_root != nullptr;
}

Bone *Skeleton::find(Bone *bone, const std::string &name) {
    if (!bone) {
        return nullptr;
    }
    if (bone->getname() == name) {
        return bone;
    }

    Bone* result = nullptr;
    for (auto& child : bone->m_children) {
        result = find(&child, name);
        if (result) {
            break;
        }
    }
    return result;
}
