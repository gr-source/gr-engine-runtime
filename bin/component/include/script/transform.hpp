#pragma once

#include "../ecs/component.hpp"

#include <gr-math/gmath.h>

#include <vector>

class Transform : public Component {
public:
    Transform();

    void setLocalScale(const Vector3 &value);
    
    void setLocalRotation(const Quaternion &value);

    const Quaternion getLocalRotation() const;

    const Quaternion getRotation() const;

    void setLocalPosition(const Vector3 &value);

    const Vector3 getLocalPosition() const;

    void addLocalPosition(const Vector3 &value);
    
    const Vector3 getPosition() const;

    const Vector3 getLocalForward() const;

    const Vector3 getForward() const;

    const Vector3 getRight() const;

    const Vector3 getUp() const;

    const Matrix4x4 &getGlobalMatrix() const;

    const Matrix4x4 &getLocalMatrix() const;

    // define o pai do objeto atual
    void setParent(Transform *transform);

    Transform *getParent();

    std::vector<Transform *> &getChildren();

    void addChildren(Transform *child);

    Transform *getChildren(const std::string &name);
    
    std::ostream &Serialize(std::ostream &os) override;

    std::istream &Deserialize(std::istream &is) override;
    
private:
    Vector3 m_scale;

    Quaternion m_rotation;

    Vector3 m_position;

    std::vector<Transform *> m_children;

    Transform* m_parent;

    Matrix4x4 m_localMatrix;

    Matrix4x4 m_globalMatrix;

    void updateLocalMatrix();

    void updateGlobalMatrix();

};

REGISTER_COMPONENT(Transform)
