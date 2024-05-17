#pragma once

#include "texture.h"

class Sprite : public Texture {
public:
    Sprite(const std::string& path = "default");
    Sprite(const GLuint& textureID, const std::string& path = "none");

    void apply(void* pixels) override;
};


