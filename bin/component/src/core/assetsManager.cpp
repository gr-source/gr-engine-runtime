#include "core/assetsManager.hpp"

#include "core/animation/animation.hpp"

#include "core/stream.hpp"
#include "core/scene.hpp"
#include "core/file.hpp"

#include "api.hpp"

template <>
grr::gTexture *AssetsManager::Load(const std::string &path, const std::string &rhs) {
    return nullptr;
}

grr::gShader *AssetsManager::LoadShader(const std::string &name, const std::string &frag, const std::string &vert) {
    return nullptr;
}

void AssetsManager::AddPostLoader(std::function<bool(void *, const std::string &)> func) {
}

Mesh *AssetsManager::FindMesh(const std::string &name) {
    return nullptr;
}

grr::gShader *AssetsManager::FindShader(const std::string &name) {
    return nullptr;
}

void AssetsManager::SetGameObjectList(const std::string &path, std::function<void(GameObject *)> &func) {}

std::vector<Material> *AssetsManager::FindMaterial(const std::string &name) {
    return nullptr;
}
