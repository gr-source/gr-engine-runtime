#include "material.h"

#include "core/assetsManager.hpp"

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

void Material::serialize(std::ostream &os) const {
    // serialize name
    auto nSize = m_name.size();
    os.write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t));
    os.write(m_name.data(), nSize * sizeof(char));

    // serialize albedo
    os.write(reinterpret_cast<const char *>(&m_albedo), sizeof(Vector4));

    // serialize metallic
    os.write(reinterpret_cast<const char *>(&m_metallic), sizeof(float));

    // serialize roughness
    os.write(reinterpret_cast<const char *>(&m_roughness), sizeof(float));

    // serialize ao
    os.write(reinterpret_cast<const char *>(&m_ao), sizeof(float));

    auto tSize = m_texture.size();
    os.write(reinterpret_cast<const char *>(&tSize), sizeof(std::size_t));

    for (int i = 0;i<m_texture.size();i++) {
        os.write(reinterpret_cast<const char *>(&i), sizeof(int));
        auto name = AssetsManager::GetTextureKeyMap(m_texture[i]);

        nSize = name.size();
        os.write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t));
        os.write(name.data(), nSize * sizeof(char));
    }
}

void Material::deserialize(std::istream &is) {
    // deserialize name
    std::size_t nSize;
    is.read(reinterpret_cast<char *>(&nSize), sizeof(std::size_t));

    m_name.resize(nSize, '\0');
    is.read(m_name.data(), nSize * sizeof(char));

    // deserialize albedo
    is.read(reinterpret_cast<char *>(&m_albedo), sizeof(Vector4));

    // deserialize metallic
    is.read(reinterpret_cast<char *>(&m_metallic), sizeof(float));

    // deserialize roughness
    is.read(reinterpret_cast<char *>(&m_roughness), sizeof(float));

    // deserialize ao
    is.read(reinterpret_cast<char *>(&m_ao), sizeof(float));

    std::size_t tSize = 0;
    is.read(reinterpret_cast<char *>(&tSize), sizeof(std::size_t));

    for (int i=0;i<tSize;i++) {
        int id = 0;
        is.read(reinterpret_cast<char *>(&id), sizeof(int));
        is.read(reinterpret_cast<char *>(&nSize), sizeof(std::size_t));

        std::string name(nSize, '\0');
        is.read(name.data(), nSize * sizeof(char));

        if (name.empty()) {
            continue;
        }
        AssetsManager::LoadTexture(name, &Material::setupTexture, this, id);
    }
}

void Material::setupTexture(int id, grr::gTexture *texture) {
    m_texture[id] = texture;
}
