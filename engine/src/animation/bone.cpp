#include "bone.h"

Bone::Bone(const std::string& name, Bone* parent) : m_name(name), m_parent(parent), id(0) {}

const std::string &Bone::getname() const {
    return m_name;
}

std::vector<Bone> &Bone::getchildren() {
    return m_children;
}

Matrix4x4 Bone::getlocalmatrix() const {
    return Math::translate(position) * Math::rotate(rotation) * Math::scale(scale);
}

const bool Bone::hasparent() const {
    return m_parent != nullptr;
}

