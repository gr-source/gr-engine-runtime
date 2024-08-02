#include "bone.hpp"

Bone::Bone(const std::string & name, const Vector3 & position, const Vector3 & scale, const Quaternion & rotation) : m_name(name), m_position(position), m_scale(scale), m_rotation(rotation) {}

const std::string &Bone::getName() const {
    return m_name;
}
