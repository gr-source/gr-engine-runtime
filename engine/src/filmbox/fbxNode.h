#pragma once

#include <string>
#include <vector>
#include <map>

#include <gr-math/gmath.h>

#include "../geometry/material.h"
#include "../geometry/mesh.h"

class FbxNode {
public:
    enum : u8 {
        eNONE   = 1 << 0,
        eCAMERA = 1 << 1,
        eLIGHT  = 1 << 2,
        eMODEL  = 1 << 3,
    };

    std::vector<FbxNode> children;
    FbxNode* parent;

    Matrix4x4 localTransform;

    Vector3 position;

    Quaternion rotation;

    Vector3 scale;

    std::unordered_map<u32, Material> materials;
    
    FbxNode(const std::string& name = "unamed", FbxNode* parent = nullptr, u8 type = eNONE);

    template <typename ...Args>
    FbxNode& addchild(Args&&... args);

    bool haschild() const;

    template <typename... Args>
    Mesh& addmesh(Args&&... args);

    bool hasmesh() const;

    const std::vector<Mesh>& getmeshs() const;

    void setname(const std::string& value);

    const std::string& getname() const;

    const u8& gettype() const;

    const std::vector<FbxNode>& getchildren() const;

protected:
    std::string name;

    std::vector<Mesh> meshs;

    u8 m_type;
};

template <typename ...Args>
inline FbxNode& FbxNode::addchild(Args&&... args) {
    return children.emplace_back(std::forward<Args>(args)...);
}

template <typename ...Args>
inline Mesh& FbxNode::addmesh(Args&&... args) {
    return meshs.emplace_back(std::forward<Args>(args)...);
}