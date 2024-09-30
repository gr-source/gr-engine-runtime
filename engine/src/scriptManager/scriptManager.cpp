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

std::unique_ptr<ScriptLoader> ScriptManager::m_script = nullptr;

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

    m_script = std::make_unique<ScriptLoader>(s_path);
    if (!m_script->isValid()) {
        throw std::runtime_error("Script do not is valid: " + path);
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

    m_script = nullptr;
}

bool ScriptManager::IsValid() {
    return m_script && m_script->isValid();
}

#include "timer.h"

// TODO: Falta corrigir error de segmebt afult em memory hook
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
void ScriptManager::RegisterApi() {
    // Console log
    m_script->replaceLibraryFunction("_ZN7Console3logEN7LogInfo7LogTypeERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", reinterpret_cast<void *>(&Console::log));

    // First function
    m_script->registerFunction("Initialize",   reinterpret_cast<void **>(&InitializeScript));
    m_script->registerFunction("Destroy",      reinterpret_cast<void **>(&Destroy));

    m_script->registerFunction("SetDeltaTime", reinterpret_cast<void **>(&SetDeltaTime));

    // shader functions
    // m_script->registerFunction("GetShader",  reinterpret_cast<void **>(&GetShader));
    m_script->registerFunction("GetShaders", reinterpret_cast<void **>(&GetShaders));

    // entity functions
    // m_script->registerFunction("_ZN6Entity6updateEv", reinterpret_cast<void **>(&Entity::Update));
    // m_script->registerFunction("_ZN6Entity4initEv",   reinterpret_cast<void **>(&Entity::Init));
    // m_script->replaceLibraryFunction("_ZN6Entity20addInternalComponentERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", reinterpret_cast<void *>(&Entity::addInternalComponent));

    // Light fundtiosn
    m_script->registerFunction("GetListLight", reinterpret_cast<void **>(&GetListLight));

    // Camera functions
    m_script->registerFunction("_ZN6Camera4MainEv",    reinterpret_cast<void **>(&Camera::Main));
    m_script->registerFunction("_ZN6Camera7CurrentEv", reinterpret_cast<void **>(&Camera::Current));
    m_script->registerFunction("GetAllCamera",    reinterpret_cast<void **>(&Camera::GetCameras));
    m_script->registerFunction("SetEnableCamera", reinterpret_cast<void **>(&Camera::SetEnable));

    // Event Manager
    m_script->replaceLibraryFunction("_ZN5Event17AddEvent_InternalESt8functionIFvvEE", reinterpret_cast<void **>(&Event::AddEvent));

    // AssetManager
    m_script->replaceLibraryFunction("_ZN13AssetsManager4LoadIN3grr8gTextureEEEPT_RKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESC_",                          reinterpret_cast<void *>(&AssetsManager::LoadTexture));
    m_script->replaceLibraryFunction("_ZN13AssetsManager17SetGameObjectListERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEERSt8functionIFvP10GameObjectEE",     reinterpret_cast<void *>(&AssetsManager::SetGameObjectList));
    m_script->replaceLibraryFunction("_ZN13AssetsManager13AddPostLoaderESt8functionIFbPvRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEEE",                     reinterpret_cast<void *>(&AssetsManager::AddPostLoader));
    m_script->replaceLibraryFunction("_ZN13AssetsManager8FindMeshERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                                             reinterpret_cast<void *>(&AssetsManager::FindMesh));
    m_script->replaceLibraryFunction("_ZN13AssetsManager12FindMaterialERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                                        reinterpret_cast<void *>(&AssetsManager::FindMaterial));
    m_script->replaceLibraryFunction("_ZN13AssetsManager10FindShaderERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                                          reinterpret_cast<void *>(&AssetsManager::FindShader));
    m_script->replaceLibraryFunction("_ZN13AssetsManager13SetShaderListERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_S7_RSt8functionIFvPN3grr7gShaderEEE", reinterpret_cast<void *>(&AssetsManager::SetShaderList));

    m_script->registerFunction("LoadAnimation", reinterpret_cast<void **>(&LoadAnimation));

    // Input
    m_script->replaceLibraryFunction("InputIsKeyPress",    reinterpret_cast<void *>(&Input::IsKeyPress));
    m_script->replaceLibraryFunction("InputSetCursourPos", reinterpret_cast<void *>(&Input::SetCursorPos));
    m_script->replaceLibraryFunction("InputGetCursorPos",  reinterpret_cast<void *>(&Input::GetCursorPos));
    // m_script->replaceLibraryFunction("InputIsMousePress", Input::IsMousePress);

    // Screen
    m_script->registerFunction("SetScreenSize", reinterpret_cast<void **>(&SetScreenSize));

    // Scene
    // m_script->replaceLibraryFunction("_ZN5Scene16CreateGameObjectERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_", Scene::CreateGameObject);
    // m_script->replaceLibraryFunction("_ZN5Scene6DeleteEP10GameObject", Scene::Delete);
    // m_script->replaceLibraryFunction("_ZN5Scene6RemoveEP10GameObject", reinterpret_cast<void **>(&Scene::Remove));

    // Component Pool
    m_script->registerFunction("GetComponentPool", reinterpret_cast<void **>(&GetComponentPool));
    m_script->registerFunction("CreateComponentWithName",   reinterpret_cast<void **>(&ScriptManager::CreateComponentWithName));
    m_script->registerFunction("CopyComponentWithName",     reinterpret_cast<void **>(&ScriptManager::CopyComponentWithName));

    // Batching
    m_script->replaceLibraryFunction("_ZN8Batching6removeEP9Transformb", (void *)&Batching::remove);
    m_script->replaceLibraryFunction("_ZN8Batching6insertEP9Transform",  (void *)&Batching::insert);

    // Script
    m_script->replaceLibraryFunction("_ZN10MeshRender5buildEv", (void *)&MeshRender::build);

    // GameObject
    m_script->replaceLibraryFunction("_ZN10GameObject11InstantiateEPS_", reinterpret_cast<void *>(&GameObject::Instantiate));
    m_script->replaceLibraryFunction("_ZN10GameObject6CreateERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_", reinterpret_cast<void *>(&GameObject::Create));
    m_script->replaceLibraryFunction("_ZN10GameObject7DestroyEPS_", reinterpret_cast<void *>(&GameObject::Destroy));

    // File
    m_script->replaceLibraryFunction("_ZN4File7GetFileERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEb", reinterpret_cast<void *>(&File::GetFile));

    // Time
    // m_script->replaceLibraryField("_ZN4Time9deltaTimeE", reinterpret_cast<void *>(&TimeLast::deltaTime));
}
#pragma GCC diagnostic pop

