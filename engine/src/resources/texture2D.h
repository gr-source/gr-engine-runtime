#pragma once

#include "texture.h"

class Texture2D : public Texture {
public:
    Texture2D();

    Texture2D(const GLuint& textureID, const std::string& path = "none");

    void apply(void* pixels = nullptr);
};

