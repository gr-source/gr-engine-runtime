#pragma once

#include <gr-render/gRender.h>

#define MAX_TEXTURE 10

class Material {
public:
    Material(const std::string& name = "Material", const Color& albedo = Color::black, float metallic = 0.5f, float roughness = 0.5f, float ao = 0.5f);

    const std::string& getName() const;

    void setTexture(int idx, grr::gTexture *texture);

    void serialize(std::ostream &os) const;

    void deserialize(std::istream &is);

private:
    std::string m_name;

    Color m_albedo;

    float m_metallic;

    float m_roughness;

    float m_ao;

    std::vector<grr::gTexture *> m_texture;
};
