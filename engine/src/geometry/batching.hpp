#pragma once

#include <gr-render/gRender.h>

#include "default.hpp"

#include "material.h"
#include "boundbox.h"

class Batching {
public:
    static grm::size GetBatchedCount();

    static Batching *Find(const std::string &name);

    static void Delete(const std::string &name);

    template <typename ...Args>
    static Batching &Add(const std::string &key, Args &&...args);

    static void Renderer();
    
    Batching(const std::string &name, std::unordered_map<int, Material> *materials, Mesh *mesh);
    ~Batching();

    void insert(Transform *transform);

    void remove(Transform *transform, bool bDelete = true);

    void draw();

    void rebuild();

    bool hasModel() const;

private:
    static std::unordered_map<std::string, std::unique_ptr<Batching>> m_batchedMap;

    static std::vector<Batching *> m_batched;

    std::unordered_map<int, Material> *m_materials;

    std::vector<Transform *> m_models;

    grr::gVertexArray *m_vertexArray;

    grm::u32 m_indiceCount;

    grm::u32 m_primCount;

    std::string m_name;

    Boundbox m_boundbox;

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
    auto &&[it, added] = m_batchedMap.emplace(key, std::make_unique<Batching>(key, std::forward<Args>(args)...));
    return *m_batched.emplace_back(&(*it->second));
}
