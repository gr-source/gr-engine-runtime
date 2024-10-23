#include "core/assetsManager.hpp"

#include "core/animation/animation.hpp"

#include "core/stream.hpp"
#include "core/scene.hpp"
#include "core/file.hpp"

#include "api.hpp"

void AssetsManager::AddPostLoader(std::function<bool(void *, const std::string &)> func) {
    throw std::runtime_error("function do not implemented.");
}

Mesh *AssetsManager::FindMesh(const std::string &name) {
    throw std::runtime_error("function do not implemented.");
}

grr::gShader *AssetsManager::FindShader(const std::string &name) {
    throw std::runtime_error("function do not implemented.");
}

void AssetsManager::SetGameObjectList(const std::string &path, std::function<void(GameObject *)> func) {
    throw std::runtime_error("function do not implemented.");
}

void AssetsManager::SetShaderList(const std::string &name, const std::string &frag, const std::string &vert, std::function<void(grr::gShader *)> func) {
    AssetsManagerSetShaderList(name, frag, vert, func);
}

void AssetsManager::SetTextureList(const std::string &path, std::function<void(grr::gTexture *)> func) {
    throw std::runtime_error("function do not implemented.");
}

std::vector<Material> *AssetsManager::FindMaterial(const std::string &name) {
    throw std::runtime_error("function do not implemented.");
}
