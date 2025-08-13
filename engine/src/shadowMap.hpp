#pragma once

#include <gr-math/gmath.hpp>

class ShadowMap {
public:
    ShadowMap() = delete;

    ShadowMap(grm::uint32 width, grm::uint32 height);
    ~ShadowMap();

    void bind();

    void unbind();

    void bindTexture(grm::uint32 index);

private:
    grm::uint32 shadowFBO;

    grm::uint32 shadowMap;
};


