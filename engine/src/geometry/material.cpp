#include "material.h"

Material::Material(const std::string& name) : m_name(name), albedo(0.8f, 0.8f, 0.8f, 1.0f), metallic(0.0f), roughness(0.32f), ao(1.0f) {
    for (u32 i=0;i<MAX_TEXTURE;i++)
        texture[i] = nullptr;
}

const std::string &Material::getname() const {
    return m_name;
}
