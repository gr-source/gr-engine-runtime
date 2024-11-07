#include "core/assetsManager.hpp"

#include "core/animation/animation.hpp"

#include "core/function.hpp"
#include "core/stream.hpp"
#include "core/scene.hpp"
#include "core/file.hpp"

// ********** Mesh ********** //
Mesh *(*AssetsManager::CreateMesh)(const std::string &key, const std::string &name);
INSERT_FUNCTION(0, AssetsManager::CreateMesh)

Mesh *(*AssetsManager::FindMesh)(const std::string &name);
INSERT_FUNCTION(1, AssetsManager::FindMesh)

grr::gShader *(*AssetsManager::FindShader)(const std::string &name);
INSERT_FUNCTION(2, AssetsManager::FindShader)

void (*AssetsManager::SetGameObjectList)(const std::string &path, std::function<void(GameObject *)> func);
INSERT_FUNCTION(3, AssetsManager::SetGameObjectList)

void (*AssetsManager::SetShaderList)(const std::string &name, const std::string &frag, const std::string &vert, std::function<void(grr::gShader *)> func);
INSERT_FUNCTION(4, AssetsManager::SetShaderList)

void (*AssetsManager::SetTextureList)(const std::string &path, std::function<void(grr::gTexture *)> func) = nullptr;
INSERT_FUNCTION(5, AssetsManager::SetTextureList)



