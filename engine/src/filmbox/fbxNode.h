#pragma once

#include "../common.h"

#include "../animation/animationClip.h"
#include "../animation/skeleton.h"

#include "../geometry/material.h"

#include "fbxMesh.h"

class FbxNode {
public:
    enum : u8 {
        eNONE   = 1 << 0,
        eCAMERA = 1 << 1,
        eLIGHT  = 1 << 2,
        eMODEL  = 1 << 3,
    };

    FbxNode* parent;

    Vector3 position;

    Quaternion rotation;

    Vector3 scale;
    
    FbxNode(const std::string& name = "unamed", FbxNode* parent = nullptr, u8 type = eNONE);

    template <typename ...Args>
    FbxNode& addchild(Args&&... args);

    bool haschild() const;

    template <typename... Args>
    FbxMesh& addmesh(Args&&... args);

    bool hasmesh() const;

    const std::vector<FbxMesh>& getmeshs() const;

    void setname(const std::string& value);

    const std::string& getname() const;

    const Matrix4x4 getGlobalMatrix() const ;

    const u8& gettype() const;

    const std::vector<FbxNode>& getchildren() const;

    const bool hasSkeleton() const;

    const Skeleton* getSkeleton() const;

    std::vector<AnimationClip> m_animations;

    template <typename... Args>
    Material& addMaterial(u32 index, Args &&...args);

    Material* findMaterial(u32 index);

    const std::unordered_map<u32, Material>& getMaterials() const;

    const u32 getMaterialCount() const;
protected:
    std::unordered_map<u32, Material> m_materials;
    
    std::vector<FbxNode> m_children;

    std::vector<FbxMesh> m_meshs;

    Skeleton* m_skeleton;

    std::string name;

    u8 m_type;

    friend class Filmbox;
    friend class Game;
};

template <typename ...Args>
inline FbxNode& FbxNode::addchild(Args&&... args) {
    return m_children.emplace_back(std::forward<Args>(args)...);
}

template <typename ...Args>
inline FbxMesh& FbxNode::addmesh(Args&&... args) {
    return m_meshs.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline Material &FbxNode::addMaterial(u32 index, Args &&...args) {
    m_materials.emplace(index, std::forward<Args>(args)...);
    return m_materials[index];
}
