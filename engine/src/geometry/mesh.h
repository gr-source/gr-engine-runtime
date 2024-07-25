#pragma once

#include <vector>

#include "../common.h"

#include "vertex3D.h"

class Mesh {
public:
    Mesh(const std::string &name = "default");

    template <typename ...Args>
    Vertex& addVertex(Args &&...args);

    void addTriangles(int t0, int t1, int t2);

    void addIndice(int t0);
private:
    std::vector<Vertex> m_vertices;

    std::vector<int> m_indices;

    std::string m_name;
};

template <typename ...Args>
inline Vertex& Mesh::addVertex(Args&& ...args) {
    return m_vertices.emplace_back(std::forward<Args>(args)...);
}
