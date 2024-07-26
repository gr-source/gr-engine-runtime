#include "material.h"

Material::Material(const std::string &name, const Vector4 &albedo, float metallic, float roughness, float ao) : m_name(name), m_albedo(albedo), m_metallic(metallic), m_roughness(roughness), m_ao(ao) {}

const std::string &Material::getName() const {
    return m_name;
}
