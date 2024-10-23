#include "core/geometry/material.hpp"

#include "api.hpp"

Material::Material(const std::string &name, const Color &albedo, float metallic, float roughness, float ao) : m_name(name), m_albedo(albedo), m_metallic(metallic), m_roughness(roughness), m_ao(ao), m_texture(10, nullptr) {}

const std::string &Material::getName() const {
    return m_name;
}

void Material::setTexture(int idx, grr::gTexture *texture) {
    if (idx >= m_texture.size()) {
        return;
    }
    m_texture[idx] = texture;
}

void Material::serialize(Stream *stream) const {
    throw std::runtime_error("function do not implemented.");
}

void Material::deserialize(Stream *stream) {
    MaterialDeserialize(this, stream);
    // throw std::runtime_error("function do not implemented.");
}

