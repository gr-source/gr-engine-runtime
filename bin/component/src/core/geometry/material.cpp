#include "core/geometry/material.hpp"

#include "core/function.hpp"

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

