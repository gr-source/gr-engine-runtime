#pragma once

#include <string>

#include "../resources/texture2D.h"

#include "../Color.h"

#define MAX_TEXTURE 10

struct Material {
    /* nome do material */
    std::string name;

    /* cor base */
    Color albedo;

    float metallic;

    float roughness;

    float ao;

    Texture2D* texture[MAX_TEXTURE];

    Material(const std::string& name = "Material");
    
    bool operator ==(const Material& other) const;
};

