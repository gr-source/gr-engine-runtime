#include "scriptManager.hpp"

#include "scriptLoader.hpp"

#include "script/meshCollider.hpp"
#include "script/boxCollider.hpp"
#include "script/meshRender.hpp"
#include "script/rigidbody.hpp"
#include "script/camera.hpp"

#include "geometry/batching.hpp"

#include "core/assetsManager.hpp"
#include "core/threadpool.hpp"
#include "core/scene.hpp"
#include "core/event.hpp"
#include "core/input.hpp"

#include "physics/physicsScene.hpp"

#include "ecs/gameObject.hpp"
#include "ecs/entity.hpp"

#include "../memory.h"

#include "settings.h"

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

// [ Entity ]
std::size_t (*ScriptManager::GetEntityCount)();

// LIght
std::vector<Light *> &(*ScriptManager::GetListLight)();

std::unique_ptr<Animation> (*ScriptManager::LoadAnimation)(const std::string &);

// Screen
void (*ScriptManager::SetScreenSize)(const iVector2 &);

// ComponentPool
std::unordered_map<std::string, Pool *> (*ScriptManager::GetComponentPool)();

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
        Console::log(Log::INFO, "Script initialized.");
        InitializeScript();
    }
}

void ScriptManager::Shutdown() {
    Console::log(Log::INFO, "Script destoyed.");

    if (Destroy) {
        Destroy();
    }

    m_script = nullptr;
}

bool ScriptManager::IsValid() {
    return m_script && m_script->isValid();
}

// TODO: Falta corrigir error de segmebt afult em memory hook
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"

bool ScriptManager::CheckScriptLoaded(const std::string &name) {
    std::ifstream is("/proc/self/maps");
    if (!is.is_open()) {
        return false;
    }
    bool isValid = false;
    
    std::string line;
    while (std::getline(is, line)) {
        if (line.find(name) == std::string::npos) {
            continue;
        }
        isValid = true;
        break;
    }
    is.close();
    return isValid;
}

void (*Consolelog)(Log::LogType, const std::string &);
void (*EventAddEvent)(std::function<void()>);

void (*AssetsManagerSetTextureList)(const std::string &, std::function<void (grr::gTexture *)>);
void (*AssetsManagerSetGameObjectList)(const std::string &, std::function<void (GameObject *)>);
Mesh *(*AssetsManagerFindMesh)(const std::string &);
grr::gShader *(*AssetsManagerFindShader)(const std::string &);
void (*AssetsManagerSetShaderList)(const std::string &, const std::string &, const std::string &, std::function<void (grr::gShader *)>);

bool (*InputIsKeyPress)(grm::u32);
void (*InputSetCursorPos)(const Vector2 &);
Vector2 (*InputGetCursorPos)();
bool (*InputIsMouseDown)(InputKey);

void (*Batchingremove)(Transform *, bool);
void (*Batchinginsert)(Transform *);

void (*MeshRenderbuild)();
GameObject &(*GameObjectInstantiate)(GameObject *);
GameObject &(*GameObjectCreate)(const std::string &, const std::string &);
void (*GameObjectDestroy)(GameObject *);

std::string (*FileGetFile)(const std::string &, bool);

const void (*Materialserialize)(Stream *);
void (*Materialdeserialize)(Stream *);

void (*MeshColliderbuild)(Mesh *);

void (*Rigidbodyprocess)();
void (*Rigidbodysetup)();

std::unordered_map<std::string, void **> (*GetFunctionsReference)();

void ScriptManager::RegisterApi() {
    // Functions Pointer
    if (!m_script->registerFunction("_ZN8Function19GetFunctionRefrenceB5cxx11Ev", reinterpret_cast<void **>(&GetFunctionsReference))) {
        return Console::LogError("Falied to register library functions.");
    }

    auto function = GetFunctionsReference();

    *function["Console::log"] = reinterpret_cast<void *>(&Console::log);
    *function["Threadpool::CreateInternalTask"] = reinterpret_cast<void *>(&Threadpool::CreateInternalTask);

    // First function
    m_script->registerFunction("Initialize",   reinterpret_cast<void **>(&InitializeScript));
    m_script->registerFunction("Destroy",      reinterpret_cast<void **>(&Destroy));

    m_script->registerFunction("SetDeltaTime", reinterpret_cast<void **>(&SetDeltaTime));
    
    // Light fundtiosn
    m_script->registerFunction("GetListLight", reinterpret_cast<void **>(&GetListLight));

    // Camera functions
    m_script->registerFunction("_ZN6Camera4MainEv",    reinterpret_cast<void **>(&Camera::Main));
    m_script->registerFunction("_ZN6Camera7CurrentEv", reinterpret_cast<void **>(&Camera::Current));
    m_script->registerFunction("GetAllCamera",    reinterpret_cast<void **>(&Camera::GetCameras));
    m_script->registerFunction("SetEnableCamera", reinterpret_cast<void **>(&Camera::SetEnable));

    // Event Manager
    *function["Event::AddInternalEvent"] = reinterpret_cast<void **>(&Event::AddEvent);

    // AssetManager
    *function["AssetsManager::SetTextureList"]    = reinterpret_cast<void *>(&AssetsManager::SetTextureList);
    *function["AssetsManager::SetGameObjectList"] = reinterpret_cast<void *>(&AssetsManager::SetGameObjectList);
    *function["AssetsManager::FindMesh"]          = reinterpret_cast<void *>(&AssetsManager::FindMesh);
    *function["AssetsManager::FindShader"]        = reinterpret_cast<void *>(&AssetsManager::FindShader);
    *function["AssetsManager::SetShaderList"]     =  reinterpret_cast<void *>(&AssetsManager::SetShaderList);
    *function["AssetsManager::CreateMesh"]        =  reinterpret_cast<void *>(&AssetsManager::CreateMesh<std::string>);

    m_script->registerFunction("LoadAnimation", reinterpret_cast<void **>(&LoadAnimation));

    // Input
    *function["Input::IskeyPress"] = reinterpret_cast<void *>(&Input::IsKeyPress);
    *function["Input::SetCursorPos"] = reinterpret_cast<void *>(&Input::SetCursorPos);
    *function["Input::GetCursorPos"] = reinterpret_cast<void *>(&Input::GetCursorPos);
    *function["Input::IsMousePress"] = reinterpret_cast<void *>(&Input::IsMouseDown);

    // Screen
    m_script->registerFunction("SetScreenSize", reinterpret_cast<void **>(&SetScreenSize));

    // Component Pool
    m_script->registerFunction("GetComponentPool", reinterpret_cast<void **>(&GetComponentPool));
    m_script->registerFunction("CreateComponentWithName",   reinterpret_cast<void **>(&ScriptManager::CreateComponentWithName));
    m_script->registerFunction("CopyComponentWithName",     reinterpret_cast<void **>(&ScriptManager::CopyComponentWithName));

    *function["Entity::addInternalComponent"] = (void *)&Entity::addInternalComponent;

    // Batching
    *function["Batching::InsertInternal"] =  (void *)&Batching::insert;
    *function["Batching::RemoveInternal"] = (void *)&Batching::remove;

    // Script
    *function["MeshRenderbuild"] = (void *)&MeshRender::build;

    // GameObject
    *function["GameObject::Instantiate"] = reinterpret_cast<void *>(&GameObject::Instantiate);
    *function["GameObject::Create"] = reinterpret_cast<void *>(&GameObject::Create);
    *function["GameObject::Find"] = reinterpret_cast<void *>(&GameObject::Find);
    *function["GameObject::Destroy"] = reinterpret_cast<void *>(&Scene::Delete);

    // File
    *function["File::GetFile"] = reinterpret_cast<void *>(&File::GetFile);

    // Material
    *function["Materialserialize"] = (void *)&Material::serialize;
    *function["Materialdeserialize"] = (void *)&Material::deserialize;

    // BoxCollider
    *function["BoxColliderbuildShape"] = (void *)&BoxCollider::build;

    // MeshCollider
    *function["MeshColliderbuild"] = (void *)&MeshCollider::build;

    // Rigidbody
    *function["RigidbodyInitialize"] = (void *)&Rigidbody::initialize;
    *function["RigidbodyUpdate"] = (void *)&Rigidbody::update;
    *function["RigidbodyShutdown"] = (void *)&Rigidbody::shutdown;

    // Raycast
    *function["Raycast::RayCast"] = reinterpret_cast<void *>(&PhysicsScene::Raycast);

    // Phys
    *function["PhysicsScene::contactTest"] = reinterpret_cast<void *>(&PhysicsScene::contactTest);

    // Mesh
    *function["Meshserialize"] = (void *)&Mesh::serialize;
    *function["Meshdeserialize"] = (void *)&Mesh::deserialize;
}
#pragma GCC diagnostic pop

