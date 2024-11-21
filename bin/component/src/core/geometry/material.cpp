#include "core/geometry/material.hpp"

#include "core/function.hpp"

Material::Material(const std::string &name, const Color &albedo, float metallic, float roughness, float ao) : m_name(name), m_albedo(albedo), m_metallic(metallic), m_roughness(roughness), m_ao(ao), m_texture(10, nullptr) {}

const std::string &Material::getName() const {
    return m_name;
}

Color Material::getAlbedo() const {
    return m_albedo;
}

void Material::setAlbedo(const Color &color) {
    m_albedo = color;
}

float Material::getMetallic() const {
    return m_metallic;
}

void Material::setMetallic(float value) {
    m_metallic = value;
}

float Material::getRoughness() const {
    return m_roughness;
}

void Material::setRoughness(float value) {
    m_roughness = value;
}

float Material::getAmbientOcclusion() const {
    return m_ao;
}

void Material::setAmbientOcclusion(float value) {
    m_ao = value;
}

void Material::setTexture(int idx, grr::gTexture *texture) {
    if (idx >= m_texture.size()) {
        throw std::runtime_error("Limite texture exceded." + std::to_string(idx) + "-" + std::to_string(m_texture.size()));
    }
    m_texture[idx] = texture;
}

grr::gTexture *Material::getTexture(int idx) const {
    if (idx >= m_texture.size()) {
        throw std::runtime_error("Id not avaliable:" + std::to_string(idx) + "/" + std::to_string(m_texture.size()));
    }
    return m_texture[idx];
}

static void (*Materialserialize)(const Material *, Stream *);
INSERT_FUNCTION(0, Materialserialize);

static void (*Materialdeserialize)(Material *, Stream *);
INSERT_FUNCTION(1, Materialdeserialize);

void Material::serialize(Stream *stream) const {
    return Materialserialize(this, stream);
}

void Material::deserialize(Stream *stream) {
    return Materialdeserialize(this, stream);
}

