#pragma once

#include <string>

#include <gr-render/gTexture.h>

class Material {
public:
    Material(const std::string& name = "Material", const Vector4& albedo = {1.0f, 1.0f, 1.0f, 1.0f}, float metallic = 0.5f, float roughness = 0.5f, float ao = 0.5f);

    const std::string& getName() const;

    const Vector4 &getAlbedo() const;

    const float getMetallic() const;
    
    const float getRoughness() const;

    const float getAmbientOcclusion() const;

    grr::gTexture *getTexture(int idx);

private:
    std::string m_name;

    Vector4 m_albedo;

    float m_metallic;

    float m_roughness;

    float m_ao;

    std::vector<grr::gTexture *> m_texture;
};

