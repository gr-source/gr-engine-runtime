#pragma once

#include "texture.h"

class Sprite : public Texture {
public:
    Sprite();
    Sprite(const GLuint& textureID, const std::string& path = "none");

    void apply(void* pixels) override;
};


