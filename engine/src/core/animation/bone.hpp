#pragma once

#include <gr-math/gmath.hpp>
#include <string>

class Bone {
public:
    Bone(const std::string &name = "default", const Vector3 &position = Vector3::zero, const Vector3 &scale = Vector3::one, const Quaternion &rotation = Quaternion::identity, grm::u32 id = 0);

    const std::string &getName() const;

    const Matrix4x4 &getLocalMatrix() const;

    const grm::u32 getBoneIndex() const;
    
private:
    std::string m_name;

    Vector3 m_scale;

    Quaternion m_rotation;

    Vector3 m_position;

    grm::u32 m_boneIndex;

    Matrix4x4 m_localMatrix;

    void updateMatrix();
};

