#pragma once

#include "../ecs/component.hpp"

#include <gr-math/gmath.h>

#include <vector>

class Transform : public Component {
public:
    Transform();

    void setLocalScale(const Vector3 &value);

    Vector3 getLocalScale() const;
    
    void setLocalRotation(const Quaternion &value);

    Quaternion getLocalRotation() const;

    Quaternion getRotation() const;

    void setLocalPosition(const Vector3 &value);

    Vector3 getLocalPosition() const;

    void addLocalPosition(const Vector3 &value);
    
    Vector3 getPosition() const;

    Vector3 getLocalForward() const;

    Vector3 getForward() const;

    Vector3 getRight() const;

    Vector3 getUp() const;

    Matrix4x4 getGlobalMatrix() const;

    Matrix4x4 getLocalMatrix() const;

    // define o pai do objeto atual
    void setParent(Transform *transform);

    Transform *getParent() const;

    std::vector<Transform *> getChildren() const;

    void addChildren(Transform *child);

    Transform *findChildren(const std::string &name) const;
    
    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;
    
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

    Transform *findChild(Transform *transform) const;
};

REGISTER_COMPONENT(Transform)
