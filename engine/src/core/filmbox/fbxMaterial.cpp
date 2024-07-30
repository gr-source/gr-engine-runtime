#include "fbxMaterial.hpp"

FbxMaterial::FbxMaterial(const std::string & name) : m_name(name) {}

const std::string &FbxMaterial::getName() const {
    return m_name;
}
