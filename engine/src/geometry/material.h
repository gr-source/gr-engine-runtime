#pragma once

#include <string>

#include "../resources/texture.h"

#include "../Color.h"

#define MAX_TEXTURE 10

class Material {
public:
    /* cor base */
    Color albedo;

    float metallic;

    float roughness;

    float ao;

    Texture* texture[MAX_TEXTURE];

    Material(const std::string& name = "Material");

    const std::string& getname() const;
private:
    std::string m_name;
};

