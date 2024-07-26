#include "material.h"

Material::Material(const std::string &name, const Vector4 &albedo, float metallic, float roughness, float ao) : m_name(name), m_albedo(albedo), m_metallic(metallic), m_roughness(roughness), m_ao(ao) {}

const std::string &Material::getName() const {
    return m_name;
}

const Vector4 &Material::getAlbedo() const {
    return m_albedo;
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
