#pragma once

#include <gr-render/gRender.h>

#include "default.hpp"

#include "material.h"

class Batching {
public:
    Batching(const std::string &name, std::unordered_map<int, Material> &materials, Mesh *mesh);
    ~Batching();

    void insert(Transform *transform);

    void remove(Transform *transform);

    void draw();

    void rebuild();

    static void Renderer();

    static Batching *Find(const std::string &name);

    static void Delete(const std::string &name);

    template <typename ...Args>
    static Batching &Add(const std::string &key, Args &&...args);

private:
    static std::unordered_map<std::string, std::unique_ptr<Batching>> m_batched;

    std::unordered_map<int, Material> m_materials;

    std::vector<Transform *> m_models;

    grr::gVertexArray *m_vertexArray;

    grm::u32 m_indiceCount;

    grm::u32 m_primCount;

    std::string m_name;

    grm::u32 m_VBO;

    grm::u32 m_EBO;

    grm::u32 m_PBO;

    bool bHasIndice;

    bool isCompiled;

    void compileMesh(Mesh *mesh);

    void compile();

    void setupMaterial();
};

template <typename... Args>
inline Batching &Batching::Add(const std::string &key, Args &&...args) {
    auto &&[it, added] = m_batched.emplace(key, std::make_unique<Batching>(key, std::forward<Args>(args)...));

    return (*it->second);
}
