#pragma once

#include <gr-render/gRender.h>

#include "default.hpp"

#include "material.h"
#include "boundbox.h"

class Batching {
public:
    enum BatchingType {
        BatchingType_Dynamic           = 0,
        BatchingType_Static_Mesh       = 1,
        BatchingType_Instanced_Dynamic = 2,
        BatchingType_Instanced_Static  = 3
    };

    static void Initialize();

    static void Shutdown();

    static void RenderShadow();
    
    static grm::size GetBatchedCount();

    static Batching *Find(const std::string &name);

    static void Delete(const std::string &name);

    template <typename ...Args>
    static Batching &Add(const std::string &key, Args &&...args);

    static void Renderer();
    
    Batching(const std::string &name, std::vector<std::shared_ptr<Material>> *materials, Mesh *mesh, grr::gShader *shader, int flags);
    ~Batching();

    void insert(Transform *transform);

    void remove(Transform *transform, bool bDelete = true);

    void draw(grr::gShader *shader = nullptr, bool bSetMaterial = true);

    bool hasModel() const;
    
    grm::u32 m_primCount;
    
private:
    enum DrawType {
        DrawType_Elements_Instanced = 1 << 1,
        DrawType_Arrays_Instanced   = 1 << 2,
        DrawType_Elements           = 1 << 3,
        DrawType_Arrays             = 1 << 4
    };

    static std::unordered_map<std::string, std::unique_ptr<Batching>> m_batchedMap;

    static std::vector<Batching *> m_batched;

    static grr::gShader *m_shaderShadow;

    std::vector<std::shared_ptr<Material>> *m_materials;

    std::vector<Transform *> m_models;

    grr::gVertexArray *m_vertexArray;

    grm::u32 m_indiceCount;

    std::string m_name;

    Boundbox m_boundbox;

    grm::u32 m_VBO;

    grm::u32 m_EBO;

    grm::u32 m_PBO;

    bool bHasIndice;

    DrawType m_type;

    Mesh *m_mesh;

    int m_flags;

    grr::gShader *m_shader;

    void build(Mesh *mesh);

    void compile();

    void buildDynamic();

    void buildDynamicInstanced();

    void buildStaticInstanced();

    void buildStaticMesh();

    void setupMaterial();

    static void RegisterShaderShadow(grr::gShader *shader);
};

template <typename... Args>
inline Batching &Batching::Add(const std::string &key, Args &&...args) {
    auto &&[it, added] = m_batchedMap.emplace(key, std::make_unique<Batching>(key, std::forward<Args>(args)...));
    assert(added && "Value subscrite.");
    
    auto ptr =  &(*it->second);
    m_batched.push_back(ptr);
    return *ptr;
}
