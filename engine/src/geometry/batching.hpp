#pragma once

#include <gr-render/gRender.h>

#include "default.hpp"

#include "material.h"

class Batching {
public:
    Batching(std::unordered_map<int, Material> &materials);
    ~Batching();

    void insert(Transform *transform, Mesh *mesh);

    void draw();

    void rebuild();

    static bool bRebuild;

    static void Renderer();

    static Batching *Find(const std::string &name);

    static void Delete(const std::string &name);

    template <typename ...Args>
    static Batching &Add(Args &&...args);

private:
    static std::unordered_map<std::string, Batching> m_batched;

    std::unordered_map<int, Material> m_materials;

    std::vector<std::tuple<Transform *, Mesh *>> m_meshes;

    grr::gVertexArray *m_vertexArray;

    grm::u32 m_count;

    grm::u32 m_VBO;

    grm::u32 m_EBO;

    bool bHasIndice;

    void compile(Mesh *mesh);

    void setupMaterial();
};

template <typename... Args>
inline Batching &Batching::Add(Args &&...args) {
    auto &&[it, added] = m_batched.emplace(std::forward<Args>(args)...);

    return it->second;
}
