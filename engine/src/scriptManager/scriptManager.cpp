#include "scriptManager.hpp"

#include "script/camera.hpp"

#include "../input.h"

#include "core/resources.h"
#include "core/scene.hpp"

#include "../ecs/entity.h"

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

    // [ Entity ]
    UNREGISTER_API(GetEntityCount);
    UNREGISTER_API(Entity::Update);

    // Light
    UNREGISTER_API(GetListLight);

    // [ Componenet - Camera ]
    UNREGISTER_API(MainCamera);
    UNREGISTER_API(CurrentCamera);
    UNREGISTER_API(Camera::GetCameras);

    UNREGISTER_API(EventUpdate);

    // Scene
    UNREGISTER_API(Scene::GetGameObject);

    UNREGISTER_API(LoadAnimation);

    UNREGISTER_API(SetScreenSize);

    // GameObject
    // UNREGISTER_API(GameObject::GetMutex);
    UNREGISTER_API(GameObject::Create);
    UNREGISTER_API(GameObject::Find);
    
    if (!s_handle) {
        return;
    }

    dlclose(s_handle);

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
    REGISTER_API_FROM_NAME("EntityUpdate", void, void, Entity::Update);

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

    // auto ptr = &Resources::Load<grr::gShader, const std::string &, const std::string &>;


    REGISTER_API(std::unique_ptr<Animation>, const std::string &, LoadAnimation);

    //Scene
    REGISTER_API_FROM_NAME("GetSceneGameObject", std::list<GameObject *> &, void, Scene::GetGameObject);

    REGISTER_API_FROM_NAME("DeleteSceneGameObject", void, GameObject *, Scene::Delete);

    REGISTER_API_FROM_NAME("SceneCreateGameObject", GameObject *, const std::string &, Scene::Create);

    // Input
    REGISTER_FUNC("InputIsKeyPress", Input::IsKeyPress);
    REGISTER_FUNC("InputSetCursourPos", Input::SetCursorPos);
    REGISTER_FUNC("InputGetCursorPos", Input::GetCursorPos);
    REGISTER_FUNC("InputIsMousePress", Input::IsMousePress);

    // Screen
    REGISTER_API(void, const iVector2 &, SetScreenSize);

    // gameobject
    REGISTER_API_FROM_NAME_N("GameObjectCreate", GameObject &, GameObject::Create, const std::string &, const std::string &);
    REGISTER_API_FROM_NAME_N("GameObjectFind", GameObject *, GameObject::Find, const std::string &);
    // REGISTER_API_FROM_NAME_N("GetGameObjectMutex", std::mutex&, GameObject::GetMutex, void);
}


