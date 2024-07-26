#pragma once

#include <string>

#include "../resources/texture.h"

#include "../Color.h"

#define MAX_TEXTURE 10

class Material {
public:
    Material(const std::string& name = "Material", const Vector4& albedo = {1.0f, 1.0f, 1.0f, 1.0f}, float metallic = 0.5f, float roughness = 0.5f, float ao = 0.5f);

    const std::string& getName() const;

private:
    std::string m_name;

    Vector4 m_albedo;

    float m_metallic;

    float m_roughness;

    float m_ao;
};

