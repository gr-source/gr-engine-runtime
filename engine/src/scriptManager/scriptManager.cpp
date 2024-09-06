#include "scriptManager.hpp"

#include "scriptLoader.hpp"

#include "script/meshRender.hpp"
#include "script/camera.hpp"

#include "geometry/batching.hpp"

#include "core/resources.h"
#include "core/scene.hpp"
#include "core/event.hpp"

#include "../ecs/entity.h"

#include "../input.h"
#include "../memory.h"

#include "settings.h"
#include "Game.h"

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

std::unordered_map<int, std::pair<const std::string, const std::string>> (*ScriptManager::GetShader)() = nullptr;

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

Component *(*ScriptManager::CopyComponentWithName)(const std::string &name, Component *component);

void ScriptManager::Initialize() {
    s_path = "/home/grsource/Documentos/AppProjects/gr-component/lib/libgr-component.so";

    if (!ScriptLoader::open(s_path)) {
        return;
    }

    // s_handle = dlopen(s_path.c_str(), RTLD_LAZY);
    // if (!s_handle) {
    //     Console::log(LogInfo::ERROR, "Falied to load Script.");
    //     return;
    // }

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

#include <sys/stat.h>

void Reload() {
    Settings::IsGameRunning =! Settings::IsGameRunning;

    if (Settings::IsGameRunning) {
        Game::UnloadScript();
    } else {
        Game::LoadScript();
    }
}

void ScriptManager::doInBackground() {
    // if (!File::Exists(s_path)) {
    //     return;
    // }

    // struct stat result;
    // if (stat(s_path.c_str(), &result) != 0) {
    //     return;
    // }

    // if (s_lastTime != result.st_mtime) {
    //     Event::Add(&Reload);
    //     s_lastTime = result.st_mtime;

    //     std::this_thread::sleep_for(std::chrono::seconds(5));

    //     if (stat(s_path.c_str(), &result) != 0) {
    //         return;
    //     }

    //     s_lastTime = result.st_mtime;
    // }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
void ScriptManager::RegisterApi() {
    // Console log
    ScriptLoader::replaceLibraryFunction("Consolelog", reinterpret_cast<void *>(&Console::log));

    // First function
    ScriptLoader::registerFunction("Initialize",   reinterpret_cast<void **>(&InitializeScript));
    ScriptLoader::registerFunction("Destroy",      reinterpret_cast<void **>(&Destroy));
    ScriptLoader::registerFunction("SetDeltaTime", reinterpret_cast<void **>(&SetDeltaTime));

    // shader functions
    ScriptLoader::registerFunction("GetShader",  reinterpret_cast<void **>(&GetShader));
    ScriptLoader::registerFunction("GetShaders", reinterpret_cast<void **>(&GetShaders));

    // entity functions
    // ScriptLoader::registerFunction("GetEntityCount", reinterpret_cast<void *>(&GetEntityCount));
    ScriptLoader::registerFunction("_ZN6Entity6updateEv", reinterpret_cast<void **>(&Entity::Update));
    ScriptLoader::registerFunction("_ZN6Entity4initEv",   reinterpret_cast<void **>(&Entity::Init));
    ScriptLoader::replaceLibraryFunction("_ZN6Entity20addInternalComponentERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", (void *)&Entity::addInternalComponent);

    // Light fundtiosn
    ScriptLoader::registerFunction("GetListLight", reinterpret_cast<void **>(&GetListLight));

    // Camera functions
    ScriptLoader::registerFunction("MainCamera",      reinterpret_cast<void **>(&Camera::Main));
    ScriptLoader::registerFunction("CurrentCamera",   reinterpret_cast<void **>(&Camera::Current));
    ScriptLoader::registerFunction("GetAllCamera",    reinterpret_cast<void **>(&Camera::GetCameras));
    ScriptLoader::registerFunction("SetEnableCamera", reinterpret_cast<void **>(&Camera::SetEnable));

    // Event Manager
    ScriptLoader::registerFunction("EventUpdate", reinterpret_cast<void **>(&EventUpdate));

    // AssetManager
    ScriptLoader::replaceLibraryFunction("LoadTexture",                                                                                                reinterpret_cast<void *>(&Resources::LoadTexture));
    ScriptLoader::replaceLibraryFunction("LoadShader",                                                                                                 reinterpret_cast<void *>(&Resources::LoadShader));
    ScriptLoader::replaceLibraryFunction("LoadGameObject",                                                                                             reinterpret_cast<void *>(&Resources::LoadGameObject));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager13AddPostLoaderESt8functionIFbPvRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEEE", reinterpret_cast<void *>(&Resources::AddPostLoader));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager8FindMeshERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                         reinterpret_cast<void *>(&Resources::FindMesh));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager12FindMaterialERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                    reinterpret_cast<void *>(&Resources::FindMaterial));
    ScriptLoader::replaceLibraryFunction("_ZN13AssetsManager10FindShaderERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                      reinterpret_cast<void *>(&Resources::FindShader));

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
    // ScriptLoader::replaceLibraryFunction("_ZN5Scene3AddEP10GameObject", Scene::Add);

    // Component Pool
    ScriptLoader::registerFunction("GetComponentPool", reinterpret_cast<void **>(&GetComponentPool));

    ScriptLoader::registerFunction("CreateComponentWithName", reinterpret_cast<void **>(&ScriptManager::CreateComponentWithName));
    ScriptLoader::registerFunction("CopyComponentWithName",   reinterpret_cast<void **>(&ScriptManager::CopyComponentWithName));

    // Batching
    ScriptLoader::replaceLibraryFunction("_ZN8Batching6removeEP9Transformb", (void *)&Batching::remove);
    ScriptLoader::replaceLibraryFunction("_ZN8Batching6insertEP9Transform",  (void *)&Batching::insert);

    // Script
    ScriptLoader::replaceLibraryFunction("_ZN10MeshRender5buildEv", (void *)&MeshRender::build);

    // GameObject
    ScriptLoader::replaceLibraryFunction("_ZN10GameObject11InstantiateEPS_", reinterpret_cast<void *>(&GameObject::Instantiate));
}
#pragma GCC diagnostic pop

