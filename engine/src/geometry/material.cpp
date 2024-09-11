#include "material.h"

#include "core/assetsManager.hpp"

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

void Material::setTexture(int idx, grr::gTexture *texture) {
    if (idx >= m_texture.size()) {
        return;
    }
    m_texture[idx] = texture;
}

grr::gTexture *Material::getTexture(int idx) const {
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
        m_texture[id] = AssetsManager::LoadTexture(name);
    }
}
