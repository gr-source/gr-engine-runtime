#pragma once

#include "../ecs/entity.h"

#include "../common.h"
#include "../buffer.h"

#include "../geometry/material.h"

class Renderer : public Component {
public:
    Renderer();
    virtual ~Renderer() override;
    
    virtual void OnInit() override;

    template <typename... Args>
    Material& addMaterial(Args&&... args);

    void buildMesh();

    void draw();
    
private:
    std::vector<Material> m_materials;

    std::unique_ptr<Buffer> m_buffer;

    static std::vector<GameObject*> objects;

    friend class RenderManager;
};

template <typename... Args>
inline Material &Renderer::addMaterial(Args &&...args) {
    return m_materials.emplace_back(std::forward<Args>(args)...);
}
