#pragma once

#include <gr-render/gRender.h>
#include <gr-math/gmath.h>

#include <string>
#include <vector>

class FbxMaterial {
public:
    FbxMaterial(const std::string &name = "default");

    Color albedo;

    float roughness;

    float metallic;

    float ao;

    const std::string &getName() const;

    void setTexture(int idx, grr::gTexture *texture);

    const grr::gTexture *getTexture(int idx) const;

private:
    std::string m_name;

    std::vector<grr::gTexture *> m_textures;
};



