#include "material.h"

Material::Material(const std::string &name, const Vector4 &albedo, float metallic, float roughness, float ao) : m_name(name), m_albedo(albedo), m_metallic(metallic), m_roughness(roughness), m_ao(ao), m_texture(10, nullptr) {}

const std::string &Material::getName() const {
    return m_name;
}

const Vector4 &Material::getAlbedo() const {
    return m_albedo;
}

void Material::setAlbedo(const Vector4 &color) {
    m_albedo = color;
}

const float Material::getMetallic() const {
    return m_metallic;
}

const float Material::getRoughness() const {
    return m_roughness;
}

const float Material::getAmbientOcclusion() const {
    return m_ao;
}

void Material::setTexture(int idx, const grr::gTexture *texture) {
    if (idx >= m_texture.size()) {
        return;
    }
    m_texture[idx] = texture;
}

const grr::gTexture *Material::getTexture(int idx) const {
    return m_texture[idx];
}
