#pragma once

#include "vertex3D.hpp"

#include "core/animation/boneWeight.hpp"

#include "core/default.hpp"

#include <vector>

class Mesh {
public:
    std::string getName() const;

    template <typename ...Args>
    Vertex3D &addVertex(Args &&...args);

    std::vector<Vertex3D> &getVertices();

    void addTriangles(int t0, int t1, int t2);

    void addIndice(int t0);

    bool hasIndices() const;

    std::vector<int> &getIndices();

    template <typename ...Args>
    BoneWeight &addBoneWeight(Args &&...args);

    std::vector<BoneWeight> getBoneWeight() const;

    void serialize(Stream *stream) const;

    void deserialize(Stream *stream);

    void clear();
    
private:
    std::vector<Vertex3D> m_vertices;

    std::vector<int> m_indices;

    std::vector<BoneWeight> m_boneWeight;

    std::string m_name;
};

template <typename... Args>
inline Vertex3D &Mesh::addVertex(Args &&...args) {
    return m_vertices.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline BoneWeight &Mesh::addBoneWeight(Args &&...args) {
    return m_boneWeight.emplace_back(std::forward<Args>(args)...);
}
