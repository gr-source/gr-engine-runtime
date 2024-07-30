#pragma once

#include <string>

#include <gr-math/gmath.h>

class FbxMaterial {
public:
    FbxMaterial(const std::string &name = "default");

    Color albedo;

    float roughness;

    float metallic;

    float ao;

    const std::string &getName() const;

private:
    std::string m_name;
};



