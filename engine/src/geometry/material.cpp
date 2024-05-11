#include "material.h"

Material::Material(const std::string& name) : name(name), albedo(0.8f, 0.8f, 0.8f, 1.0f), metallic(0.0f), roughness(0.32f), ao(1.0f) {
    for (u32 i=0;i<MAX_TEXTURE;i++)
        texture[i] = nullptr;
}

bool Material::operator ==(const Material& other) const {
    return this == &other;
}
