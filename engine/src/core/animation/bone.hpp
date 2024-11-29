#pragma once

#include <gr-math/gmath.hpp>
#include <string>

class Bone {
public:
    Bone(const std::string &name = "default", const Vector3 &position = Vector3::zero, const Vector3 &scale = Vector3::one, const Quaternion &rotation = Quaternion::identity, int id = 0);

    std::string getName() const;

    Matrix4x4 getLocalMatrix() const;

    int getBoneIndex() const;
    
private:
    std::string m_name;

    Vector3 m_scale;

    Quaternion m_rotation;

    Vector3 m_position;

    int m_boneIndex;

    Matrix4x4 m_localMatrix;

    void updateMatrix();
};

