#pragma once

#include <string>
#include <vector>

#include <gr-math/gmath.h>

/* SkeletonBone */
class Bone {
public:
    int id;

    Vector3 position;

    Quaternion rotation;
    
    Vector3 scale;

    Matrix4x4 getlocalmatrix() const;

    Bone(const std::string& name = "none", Bone* parent = nullptr);

    const std::string& getname() const;

    void setname(const std::string& name);

    template <typename... Args>
    Bone& addchild(Args&&... args);

    const bool haschildren() const;

    const std::vector<Bone>& getchildren() const;

    const bool hasparent() const;

protected:
    std::vector<Bone> m_children;

    std::string m_name;

    Bone* m_parent;

    friend class Skeleton;
    friend class Filmbox;
};

template <typename... Args>
inline Bone &Bone::addchild(Args &&...args) {
    return m_children.emplace_back(std::forward<Args>(args)...);
}
