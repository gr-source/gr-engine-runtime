#pragma once

#include <gr-math/gmath.h>

#include <string>

class FbxBone {
public:
    FbxBone(const std::string &name, int id);

    const std::string &getName() const;

    void setPosition(const Vector3 &position);

    const Vector3 &getPosition() const;

    void setScale(const Vector3 &scale);

    const Vector3 &getScale() const;

    void setRotation(const Quaternion &rotation);

    const Quaternion &getRotation() const;

    int m_id;

private:
    std::string m_name;

    Vector3 m_position;

    Vector3 m_scale;
    
    Quaternion m_rotation;
};





