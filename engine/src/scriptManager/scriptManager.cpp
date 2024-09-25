#include "scriptManager.hpp"

#include "scriptLoader.hpp"

#include "script/meshRender.hpp"
#include "script/camera.hpp"

#include "geometry/batching.hpp"

#include "core/assetsManager.hpp"
#include "core/scene.hpp"
#include "core/event.hpp"

#include "../ecs/entity.h"

#include "../input.h"
#include "../memory.h"

#include "settings.h"
// #include "Game.h"

#include <iostream>
#include <fstream>
#include <dlfcn.h>

grm::size ScriptManager::s_lastTime = 0;

std::string ScriptManager::s_path = "";

bool ScriptManager::s_bReload = false;

// [ Main ]
void (*ScriptManager::InitializeScript)();
void (*ScriptManager::Update)();
void (*ScriptManager::Destroy)();
void (*ScriptManager::SetDeltaTime)(float);

// std::unordered_map<int, std::pair<const std::string, const std::string>> (*ScriptManager::GetShader)() = nullptr;

std::unordered_map<int, grr::gShader *> (*ScriptManager::GetShaders)();

// [ Entity ]
std::size_t (*ScriptManager::GetEntityCount)();

// LIght
std::vector<Light *> &(*ScriptManager::GetListLight)();

// Event
void (*ScriptManager::EventUpdate)();

std::unique_ptr<Animation> (*ScriptManager::LoadAnimation)(const std::string &);

// Screen
void (*ScriptManager::SetScreenSize)(const iVector2 &);

// ComponentPool
std::unordered_map<std::string, std::unique_ptr<Pool>> &(*ScriptManager::GetComponentPool)();

Component *(*ScriptManager::CreateComponentWithName)(const std::string &name);

Component *(*ScriptManager::CopyComponentWithName)(const std::string &name, const Component *component);

void ScriptManager::Initialize(const std::string &path) {
    if (!File::Exists(path)) {
        throw std::runtime_error("Script data do not exists: " + path);
    }
    s_path = path;

    if (!ScriptLoader::open(s_path)) {
        return;
    }

    RegisterApi();

    if (InitializeScript) {
        Console::log(LogInfo::INFO, "Script initialized.");
        InitializeScript();
    }
}

void ScriptManager::Shutdown() {
    Console::log(LogInfo::INFO, "Script destoyed.");

    if (Destroy) {
        Destroy();
    }

    ScriptLoader::close();
}

#include "timer.h"

// TODO: Falta corrigir error de segmebt afult em memory hook
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
void ScriptManager::RegisterApi() {
    // Console log
    ScriptLoader::replaceLibraryFunction("_ZN7Console3logEN7LogInfo7LogTypeERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", reinterpret_cast<void *>(&Console::log));

    // First function
    ScriptLoader::registerFunction("Initialize",   reinterpret_cast<void **>(&InitializeScript));
    ScriptLoader::registerFunction("Destroy",      reinterpret_cast<void **>(&Destroy));

    ScriptLoader::registerFunction("SetDeltaTime", reinterpret_cast<void **>(&SetDeltaTime));

    // shader functions
    // ScriptLoader::registerFunction("GetShader",  reinterpret_cast<void **>(&GetShader));
    ScriptLoader::registerFunction("GetShaders", reinterpret_cast<void **>(&GetShaders));

    // entity functions
    // ScriptLoader::registerFunction("_ZN6Entity6updateEv", reinterpret_cast<void **>(&Entity::Update));
    // ScriptLoader::registerFunction("_ZN6Entity4initEv",   reinterpret_cast<void **>(&Entity::Init));
    // ScriptLoader::replaceLibraryFunction("_ZN6Entity20addInternalComponentERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", reinterpret_cast<void *>(&Entity::addInternalComponent));

    // Light fundtiosn
    ScriptLoader::registerFunction("GetListLight", reinterpret_cast<void **>(&GetListLight));

    // Camera functions
    ScriptLoader::registerFunction("_ZN6Camera4MainEv",    reinterpret_cast<void **>(&Camera::Main));
    ScriptLoader::registerFunction("_ZN6Camera7CurrentEv", reinterpret_cast<void **>(&Camera::Current));
    ScriptLoader::registerFunction("GetAllCamera",    reinterpret_cast<void **>(&Camera::GetCameras));
    ScriptLoader::registerFunction("SetEnableCamera", reinterpret_cast<void **>(&Camera::SetEnable));

    // Event Manager
    ScriptLoader::registerFunction("EventUpdate", reinterpret_cast<void **>(&EventUpdate));

    // AssetManager
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager4LoadIN3grr8gTextureEEEPT_RKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESC_",                      reinterpret_cast<void *>(&AssetsManager::LoadTexture));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager17SetGameObjectListERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEERSt8functionIFvP10GameObjectEE", reinterpret_cast<void *>(&AssetsManager::SetGameObjectList));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager13AddPostLoaderESt8functionIFbPvRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEEE",                 reinterpret_cast<void *>(&AssetsManager::AddPostLoader));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager8FindMeshERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                                         reinterpret_cast<void *>(&AssetsManager::FindMesh));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager12FindMaterialERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                                    reinterpret_cast<void *>(&AssetsManager::FindMaterial));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager10FindShaderERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                                      reinterpret_cast<void *>(&AssetsManager::FindShader));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager10LoadShaderERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_S7_",                                reinterpret_cast<void *>(&AssetsManager::LoadShader));

    ScriptLoader::registerFunction("LoadAnimation", reinterpret_cast<void **>(&LoadAnimation));

    // Input
    ScriptLoader::replaceLibraryFunction("InputIsKeyPress",    reinterpret_cast<void *>(&Input::IsKeyPress));
    ScriptLoader::replaceLibraryFunction("InputSetCursourPos", reinterpret_cast<void *>(&Input::SetCursorPos));
    ScriptLoader::replaceLibraryFunction("InputGetCursorPos",  reinterpret_cast<void *>(&Input::GetCursorPos));
    // ScriptLoader::replaceLibraryFunction("InputIsMousePress", Input::IsMousePress);

    // Screen
    ScriptLoader::registerFunction("SetScreenSize", reinterpret_cast<void **>(&SetScreenSize));

    // Scene
    // ScriptLoader::replaceLibraryFunction("_ZN5Scene16CreateGameObjectERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_", Scene::CreateGameObject);
    // ScriptLoader::replaceLibraryFunction("_ZN5Scene6DeleteEP10GameObject", Scene::Delete);
    // ScriptLoader::replaceLibraryFunction("_ZN5Scene6RemoveEP10GameObject", reinterpret_cast<void **>(&Scene::Remove));

    // Component Pool
    ScriptLoader::registerFunction("GetComponentPool", reinterpret_cast<void **>(&GetComponentPool));
    ScriptLoader::registerFunction("CreateComponentWithName",   reinterpret_cast<void **>(&ScriptManager::CreateComponentWithName));
    ScriptLoader::registerFunction("CopyComponentWithName",     reinterpret_cast<void **>(&ScriptManager::CopyComponentWithName));

    // Batching
    ScriptLoader::replaceLibraryFunction("_ZN8Batching6removeEP9Transformb", (void *)&Batching::remove);
    ScriptLoader::replaceLibraryFunction("_ZN8Batching6insertEP9Transform",  (void *)&Batching::insert);

    // Script
    ScriptLoader::replaceLibraryFunction("_ZN10MeshRender5buildEv", (void *)&MeshRender::build);

    // GameObject
    ScriptLoader::replaceLibraryFunction("_ZN10GameObject11InstantiateEPS_", reinterpret_cast<void *>(&GameObject::Instantiate));
    ScriptLoader::replaceLibraryFunction("_ZN10GameObject6CreateERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_", reinterpret_cast<void *>(&GameObject::Create));
    ScriptLoader::replaceLibraryFunction("_ZN10GameObject7DestroyEPS_", reinterpret_cast<void *>(&GameObject::Destroy));

    // File
    // ScriptLoader::replaceLibraryFunction("_ZN4File7GetFileERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEb", reinterpret_cast<void *>(&File::GetFile));

    // Time
    // ScriptLoader::replaceLibraryField("_ZN4Time9deltaTimeE", reinterpret_cast<void *>(&TimeLast::deltaTime));
}
#pragma GCC diagnostic pop

