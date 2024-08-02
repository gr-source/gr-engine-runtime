#pragma once

#include <gr-math/gmath.h>
#include <string>

class Bone {
public:
    Bone(const std::string &name, const Vector3 &position, const Vector3 &scale, const Quaternion &rotation);

    const std::string &getName() const;
private:
    std::string m_name;

    Vector3 m_position;

    Vector3 m_scale;

    Quaternion m_rotation;
};

