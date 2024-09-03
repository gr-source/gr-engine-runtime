#include "scriptManager.hpp"

#include "script/meshRender.hpp"
#include "script/camera.hpp"

#include "geometry/batching.hpp"

#include "core/resources.h"
#include "core/scene.hpp"

#include "../ecs/entity.h"

#include "../input.h"
#include "../memory.h"

#include <iostream>
#include <dlfcn.h>

#define REGISTER_API(r, p, f) \
    f = (r(*)(p))dlsym(ScriptManager::s_handle, #f); \
    if (!f) { \
        std::cout << "Falied to register function: " << #f << std::endl; \
    } \

#define REGISTER_API_FROM_NAME(n, r, p, f) \
    f = (r(*)(p))dlsym(ScriptManager::s_handle, n); \
    if (!f) { \
        std::cout << "Falied to register function: " << #f << std::endl; \
    } \

#define REGISTER_API_FROM_NAME_N(n, r, f, ...) \
    f = (r(*)(__VA_ARGS__))dlsym(ScriptManager::s_handle, n); \
    if (!f) { \
        std::cout << "Falied to register function: " << #f << std::endl; \
    } \

#define REGISTER_FUNC(symbol, hookFunc) Memory::Hook(dlsym(ScriptManager::s_handle, symbol), (void *)&hookFunc)

#define UNREGISTER_API(func) func = nullptr

#define REGISTER_FIELD(n, f, ...) \
    f = ((__VA_ARGS__ *)dlsym(ScriptManager::s_handle, n))

// #define REGISTER_FIELD(n, f, ) \
    // f = ((__VA_ARGS__ *)dlsym(ScriptManager::s_handle, n))

void* ScriptManager::s_handle = nullptr;

// [ Main ]
void (*ScriptManager::InitializeScript)();
void (*ScriptManager::Update)();
void (*ScriptManager::Destroy)();
void (*ScriptManager::SetDeltaTime)(float);

std::unordered_map<int, std::pair<const std::string, const std::string>> (*ScriptManager::GetShader)();

std::unordered_map<int, grr::gShader *> (*ScriptManager::GetShaders)();

// [ Entity ]
std::size_t (*ScriptManager::GetEntityCount)();

// LIght
std::vector<Light *> &(*ScriptManager::GetListLight)();

// [ Componenet - Camera ]
Camera *(*ScriptManager::MainCamera)();
Camera *(*ScriptManager::CurrentCamera)();

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
    Console::log(LogInfo::INFO, "Loading script...");

    s_handle = dlopen("/home/grsource/Documentos/AppProjects/gr-component/lib/libgr-component.so", RTLD_LAZY);
    if (!s_handle) {
        Console::log(LogInfo::ERROR, "Falied to load Script.");
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

    // [ Main ]
    UNREGISTER_API(InitializeScript);

    UNREGISTER_API(Update);
    UNREGISTER_API(Destroy);
    UNREGISTER_API(SetDeltaTime);

    UNREGISTER_API(GetShader);
    UNREGISTER_API(GetShaders);

    // [ Entity ]
    UNREGISTER_API(GetEntityCount);
    UNREGISTER_API(Entity::Update);
    UNREGISTER_API(Entity::Init);

    // Light
    UNREGISTER_API(GetListLight);

    // [ Componenet - Camera ]
    UNREGISTER_API(MainCamera);
    UNREGISTER_API(CurrentCamera);
    UNREGISTER_API(Camera::GetCameras);
    UNREGISTER_API(Camera::SetEnable);

    // Event
    UNREGISTER_API(EventUpdate);

    // AssetManager
    UNREGISTER_API(LoadAnimation);

    // Screen
    UNREGISTER_API(SetScreenSize);

    // Component Pool
    UNREGISTER_API(GetComponentPool);

    UNREGISTER_API(ScriptManager::CreateComponentWithName);
    UNREGISTER_API(ScriptManager::CopyComponentWithName);

    if (!s_handle) {
        return;
    }

    if (dlclose(s_handle) != 0) {
        Console::log(LogInfo::ERROR, "Failed to flush script data: " + std::string(dlerror()));
    }

    s_handle = nullptr;
}

void ScriptManager::RegisterApi() {
    // Functions
    REGISTER_FUNC("Consolelog", Console::log);

    // [ Main ]
    REGISTER_API_FROM_NAME_N("Initialize", void, InitializeScript, void);

    REGISTER_API(void, void, Update);
    REGISTER_API(void, void, Destroy);
    REGISTER_API(void, float, SetDeltaTime);

    GetShader = (std::unordered_map<int, std::pair<const std::string, const std::string>>(*)(void))dlsym(ScriptManager::s_handle, "GetShader");
    GetShaders = (std::unordered_map<int, grr::gShader *>(*)(void))dlsym(ScriptManager::s_handle, "GetShaders");

    // REGISTER_API_FROM_NAME_N("GetShaders", std::unordered_map<int, grr::gShader *>, GetShaders, void);

    // [ Entity ]
    REGISTER_API(std::size_t, void, GetEntityCount);
    REGISTER_API_FROM_NAME("_ZN6Entity6updateEv", void, Entity *, Entity::Update);
    REGISTER_API_FROM_NAME("_ZN6Entity4initEv", void, Entity *, Entity::Init);
    REGISTER_API_FROM_NAME_N("_ZN6Entity9setEnableEb", void, Entity::SetEnable, Entity *, bool);

    REGISTER_FUNC("_ZN6Entity20addInternalComponentERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", Entity::addInternalComponent);

    // Light
    REGISTER_API(std::vector<Light *> &, void, GetListLight);

    // [ Componenet - Camera ]
    REGISTER_API(Camera*, void, MainCamera);
    REGISTER_API(Camera*, void, CurrentCamera);
    REGISTER_API_FROM_NAME("GetAllCamera", std::vector<Camera *> &, void, Camera::GetCameras);
    REGISTER_API_FROM_NAME("SetEnableCamera", void, Camera *, Camera::SetEnable);

    // Event
    REGISTER_API(void, void, EventUpdate);

    // AssetManager
    REGISTER_FUNC("LoadTexture", Resources::LoadTexture);
    REGISTER_FUNC("LoadShader", Resources::LoadShader);
    REGISTER_FUNC("LoadGameObject", Resources::LoadGameObject);
    REGISTER_FUNC("_ZN13AssetsManager13AddPostLoaderESt8functionIFbPvRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEEE", Resources::AddPostLoader);
    REGISTER_FUNC("_ZN13AssetsManager8FindMeshERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", Resources::FindMesh);
    REGISTER_FUNC("_ZN13AssetsManager12FindMaterialERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE", Resources::FindMaterial);

    REGISTER_API(std::unique_ptr<Animation>, const std::string &, LoadAnimation);

    // Input
    REGISTER_FUNC("InputIsKeyPress", Input::IsKeyPress);
    REGISTER_FUNC("InputSetCursourPos", Input::SetCursorPos);
    REGISTER_FUNC("InputGetCursorPos", Input::GetCursorPos);
    // REGISTER_FUNC("InputIsMousePress", Input::IsMousePress);

    // Screen
    REGISTER_API(void, const iVector2 &, SetScreenSize);

    // Scene
    REGISTER_FUNC("_ZN5Scene16CreateGameObjectERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES7_", Scene::CreateGameObject);
    REGISTER_FUNC("_ZN5Scene6DeleteEP10GameObject", Scene::Delete);
    REGISTER_FUNC("_ZN5Scene3AddEP10GameObject", Scene::Add);

    // Component Pool
    GetComponentPool = (std::unordered_map<std::string, std::unique_ptr<Pool>> &(*)(void))dlsym(ScriptManager::s_handle, "GetComponentPool");

    REGISTER_API_FROM_NAME_N("CreateComponentWithName", Component *, ScriptManager::CreateComponentWithName, const std::string &);
    REGISTER_API_FROM_NAME_N("CopyComponentWithName", Component *, ScriptManager::CopyComponentWithName, const std::string &, Component *);

    // Batching
    REGISTER_FUNC("_ZN8Batching6removeEP9Transformb", Batching::remove);
    REGISTER_FUNC("_ZN8Batching6insertEP9Transform", Batching::insert);

    // Script
    REGISTER_FUNC("_ZN10MeshRender5buildEv", MeshRender::build);
}


