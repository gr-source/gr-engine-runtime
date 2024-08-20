#include "bone.hpp"

Bone::Bone(const std::string &name, const Vector3 &position, const Vector3 &scale, const Quaternion &rotation, grm::u32 id) : m_name(name), m_position(position), m_scale(scale), m_rotation(rotation), m_boneIndex(id), m_localMatrix() {
    updateMatrix();
}

const std::string &Bone::getName() const {
    return m_name;
}

const Matrix4x4 &Bone::getLocalMatrix() const {
    return m_localMatrix;
}

const grm::u32 Bone::getBoneIndex() const {
    return m_boneIndex;
}

void Bone::updateMatrix() {
    m_localMatrix = Math::translate<Matrix4x4>(m_position) * Math::rotate<Matrix4x4>(Math::normalize(m_rotation)) * Math::scale<Matrix4x4>(m_scale);
}
