#include "scriptManager.hpp"

#include "scriptLoader.hpp"

#include "script/meshCollider.hpp"
#include "script/boxCollider.hpp"
#include "script/meshRender.hpp"
#include "script/rigidbody.hpp"
#include "script/camera.hpp"

#include "geometry/batching.hpp"

#include "core/assetsManager.hpp"
#include "core/scene.hpp"
#include "core/event.hpp"
#include "core/input.hpp"

#include "physics/physicsScene.hpp"

#include "../ecs/entity.h"

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

void (*Consolelog)(LogInfo::LogType, const std::string &);
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

const void (*Materialserialize)(Streaming *);
void (*Materialdeserialize)(Streaming *);

void (*MeshColliderbuild)(Mesh *);

void (*Rigidbodyprocess)();
void (*Rigidbodysetup)();


void ScriptManager::RegisterApi() {
    // Console log
    m_script->replaceLibraryFunction("_ZN7Console3logEN7LogInfo7LogTypeERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", reinterpret_cast<void *>(&Console::log), (void **)&Consolelog);

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
    m_script->replaceLibraryFunction("_ZN5Event17AddEvent_InternalESt8functionIFvvEE", reinterpret_cast<void **>(&Event::AddEvent), (void **)&EventAddEvent);

    // AssetManager
    m_script->replaceLibraryFunction("_ZN13AssetsManager14SetTextureListERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESt8functionIFvPN3grr8gTextureEEE",      reinterpret_cast<void *>(&AssetsManager::SetTextureList), (void **)&AssetsManagerSetTextureList);
    m_script->replaceLibraryFunction("_ZN13AssetsManager17SetGameObjectListERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESt8functionIFvP10GameObjectEE",      reinterpret_cast<void *>(&AssetsManager::SetGameObjectList), (void **)&AssetsManagerSetGameObjectList);
    m_script->replaceLibraryFunction("_ZN13AssetsManager8FindMeshERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                                             reinterpret_cast<void *>(&AssetsManager::FindMesh), (void **)&AssetsManagerFindMesh);
    m_script->replaceLibraryFunction("_ZN13AssetsManager10FindShaderERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",                                          reinterpret_cast<void *>(&AssetsManager::FindShader), (void **)&AssetsManagerFindShader);
    m_script->replaceLibraryFunction("_ZN13AssetsManager13SetShaderListERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_S7_St8functionIFvPN3grr7gShaderEEE",  reinterpret_cast<void *>(&AssetsManager::SetShaderList), (void **)&AssetsManagerSetShaderList);

    m_script->registerFunction("LoadAnimation", reinterpret_cast<void **>(&LoadAnimation));

    // Input
    m_script->replaceLibraryFunction("_ZN5Input10IskeyPressE8InputKey",    reinterpret_cast<void *>(&Input::IsKeyPress),    (void **)&InputIsKeyPress);
    m_script->replaceLibraryFunction("_ZN5Input12SetCursorPosERK7Vector2", reinterpret_cast<void *>(&Input::SetCursorPos), (void **)&InputSetCursorPos);
    m_script->replaceLibraryFunction("_ZN5Input12GetCursorPosEv",          reinterpret_cast<void *>(&Input::GetCursorPos), (void **)&InputGetCursorPos);
    m_script->replaceLibraryFunction("_ZN5Input12IsMousePressE8InputKey",  reinterpret_cast<void *>(&Input::IsMouseDown), (void **)&InputIsMouseDown);

    // Screen
    m_script->registerFunction("SetScreenSize", reinterpret_cast<void **>(&SetScreenSize));

    // Component Pool
    m_script->registerFunction("GetComponentPool", reinterpret_cast<void **>(&GetComponentPool));
    m_script->registerFunction("CreateComponentWithName",   reinterpret_cast<void **>(&ScriptManager::CreateComponentWithName));
    m_script->registerFunction("CopyComponentWithName",     reinterpret_cast<void **>(&ScriptManager::CopyComponentWithName));
    m_script->replaceLibraryFunction("_ZN6Entity20addInternalComponentERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", (void *)&Entity::addInternalComponent, (void **)&Batchingremove);

    // Batching
    m_script->replaceLibraryFunction("_ZN8Batching6removeEP9Transformb", (void *)&Batching::remove, (void **)&Batchingremove);
    m_script->replaceLibraryFunction("_ZN8Batching6insertEP9Transform",  (void *)&Batching::insert, (void **)&Batchinginsert);

    // Script
    m_script->replaceLibraryFunction("_ZN10MeshRender5buildEv", (void *)&MeshRender::build, (void **)&MeshRenderbuild);

    // GameObject
    m_script->replaceLibraryFunction("_ZN10GameObject11InstantiateEPS_", reinterpret_cast<void *>(&GameObject::Instantiate), (void **)&GameObjectInstantiate);
    m_script->replaceLibraryFunction("_ZN10GameObject6CreateERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_", reinterpret_cast<void *>(&GameObject::Create), (void **)&GameObjectCreate);
    m_script->replaceLibraryFunction("_ZN10GameObject7DestroyEPS_", reinterpret_cast<void *>(&GameObject::Destroy), (void **)&GameObjectDestroy);

    // File
    m_script->replaceLibraryFunction("_ZN4File7GetFileERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEb", reinterpret_cast<void *>(&File::GetFile), (void **)&FileGetFile);

    // Material
    m_script->replaceLibraryFunction("_ZNK8Material9serializeEP6Stream", (void *)&Material::serialize, (void **)&Materialserialize);
    m_script->replaceLibraryFunction("_ZN8Material11deserializeEP6Stream", (void *)&Material::deserialize, (void **)&Materialdeserialize);

    // BoxCollider
    m_script->replaceLibraryFunction("_ZN11BoxCollider10buildShapeERK7Vector3", (void *)&BoxCollider::build, (void **)&MeshColliderbuild);

    // MeshCollider
    m_script->replaceLibraryFunction("_ZN12MeshCollider5buildEP4Mesh", (void *)&MeshCollider::build, (void **)&MeshColliderbuild);

    // Rigidbody
    m_script->replaceLibraryFunction("_ZN9Rigidbody10initializeEv", (void *)&Rigidbody::initialize, (void **)&Rigidbodysetup);
    m_script->replaceLibraryFunction("_ZN9Rigidbody8shutdownEv", (void *)&Rigidbody::shutdown, (void **)&Rigidbodyprocess);
    m_script->replaceLibraryFunction("_ZN9Rigidbody6updateEv", (void *)&Rigidbody::update, (void **)&Rigidbodyprocess);

    // Raycast
    m_script->replaceLibraryFunction("_ZN7Raycast7RayCastERK7Vector3S2_", (void *)&PhysicsScene::Raycast, (void **)&Rigidbodyprocess);
}
#pragma GCC diagnostic pop

