#include "fbxMaterial.hpp"

FbxMaterial::FbxMaterial(const std::string & name) : m_name(name), m_textures(10, nullptr) {}

const std::string &FbxMaterial::getName() const {
    return m_name;
}

void FbxMaterial::setTexture(int idx, grr::gTexture *texture) {
    if (idx >= m_textures.size()) {
        throw std::runtime_error("[ FbxMaterial ] Falied to set texture. id=" + std::to_string(idx));
    }
    m_textures[idx] = texture;
}

const grr::gTexture *FbxMaterial::getTexture(int idx) const {
    if (idx >= m_textures.size()) {
        throw std::runtime_error("[ FbxMaterial ] Falied to get texture. id=" + std::to_string(idx));
    }
    return m_textures[idx];
}
