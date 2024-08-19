#pragma once

#include <vector>

#include "../common.h"

#include "vertex3D.h"

#include "../core/animation/boneWeight.hpp"

class Mesh {
public:
    Mesh(const std::string &name = "default");

    void setName(const std::string &name);

    const std::string &getName() const;

    template <typename ...Args>
    Vertex3D& addVertex(Args &&...args);

    std::vector<Vertex3D> &getVertices();

    void addTriangles(int t0, int t1, int t2);

    void addIndice(int t0);

    std::vector<int> &getIndices();

    const bool hasIndices() const;

    template <typename ...Args>
    BoneWeight &addBoneWeight(Args &&...args);

    const std::vector<BoneWeight> &getBoneWeight() const;

private:
    std::vector<Vertex3D> m_vertices;

    std::vector<int> m_indices;

    std::vector<BoneWeight> m_boneWeight;

    std::string m_name;
};

template <typename ...Args>
inline Vertex3D& Mesh::addVertex(Args&& ...args) {
    return m_vertices.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline BoneWeight &Mesh::addBoneWeight(Args &&...args) {
    return m_boneWeight.emplace_back(std::forward<Args>(args)...);
}


