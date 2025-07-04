#pragma once

#include <gr-math/gmath.hpp>

class ShadowMap {
public:
    ShadowMap() = delete;

    ShadowMap(grm::u32 width, grm::u32 height);
    ~ShadowMap();

    void bind();

    void unbind();

    void bindTexture(grm::u32 index);

private:
    grm::u32 shadowFBO;

    grm::u32 shadowMap;
};


