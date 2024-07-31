#include "scriptManager.hpp"

#include "../resources/resources.h"
#include "../ecs/entity.h"
    #include "../core/scene.hpp"

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

#define REGISTER_FUNC(symbol, hookFunc) Memory::Hook(dlsym(ScriptManager::s_handle, symbol), (void *)&hookFunc)

#define UNREGISTER_API(func) func = nullptr

void* ScriptManager::s_handle = nullptr;

// [ Main ]
void (*ScriptManager::Initialize)();
void (*ScriptManager::Update)();
void (*ScriptManager::Destroy)();
void (*ScriptManager::SetDeltaTime)(float);

std::unordered_map<int, std::pair<const std::string, const std::string>> (*ScriptManager::GetShader)();

std::vector<std::unique_ptr<GameObject>> &(*ScriptManager::GetAllGameObject)();

// [ Entity ]
std::size_t (*ScriptManager::GetEntityCount)();

// LIght
std::vector<Light *> &(*ScriptManager::GetListLight)();

// [ Component - MeshRender ]
std::vector<GameObject*> &(*ScriptManager::GetGameObjectsFromRender)(); 

// [ Componenet - Camera ]
Camera *(*ScriptManager::MainCamera)();
Camera *(*ScriptManager::CurrentCamera)();

// Event
void (*ScriptManager::EventUpdate)();

void ScriptManager::Initialzie() {
    Console::Print(LogInfo::INFO, "Loading script...");

    s_handle = dlopen("/home/grsource/Documentos/AppProjects/gr-component/lib/libgr-component.so", RTLD_LAZY);
    if (!s_handle) {
        Console::Print(LogInfo::ERROR, "Falied to load Script.");
        return;
    }

    RegisterApi();

    if (Initialize) {
        Console::Print(LogInfo::INFO, "Script initialized.");
        Initialize();
    }
}

void ScriptManager::Shutdown() {
    Console::Print(LogInfo::INFO, "Script destoyed.");

    if (Destroy) {
        Destroy();
    }

    // [ Main ]
    UNREGISTER_API(Initialize);
    UNREGISTER_API(Update);
    UNREGISTER_API(Destroy);
    UNREGISTER_API(SetDeltaTime);

    // [ Component - MeshRender ]
    UNREGISTER_API(GetGameObjectsFromRender);
    UNREGISTER_API(GetAllGameObject);

    // [ Entity ]
    UNREGISTER_API(GetEntityCount);
    UNREGISTER_API(Entity::Update);

    // Light
    UNREGISTER_API(GetListLight);

    // [ Componenet - Camera ]
    UNREGISTER_API(MainCamera);
    UNREGISTER_API(CurrentCamera);

    UNREGISTER_API(EventUpdate);

    // Scene
    UNREGISTER_API(Scene::GetGameObject);
    
    if (!s_handle) {
        return;
    }

    dlclose(s_handle);

    s_handle = nullptr;
}

void ScriptManager::RegisterApi() {
    // Functions
    REGISTER_FUNC("ConsolePrint", Console::Print);

    // [ Main ]
    REGISTER_API(void, void, Initialize);
    REGISTER_API(void, void, Update);
    REGISTER_API(void, void, Destroy);
    REGISTER_API(void, float, SetDeltaTime);

    GetShader = (std::unordered_map<int, std::pair<const std::string, const std::string>>(*)(void))dlsym(ScriptManager::s_handle, "GetShader");

    // [ Component - MeshRender ]
    REGISTER_API(std::vector<GameObject*>&, void, GetGameObjectsFromRender);
    REGISTER_API(std::vector<std::unique_ptr<GameObject>>&, void, GetAllGameObject);

    // [ Entity ]
    REGISTER_API(std::size_t, void, GetEntityCount);
    REGISTER_API_FROM_NAME("EntityProcess", void, void, Entity::Update);

    // Light
    REGISTER_API(std::vector<Light *> &, void, GetListLight);

    // [ Componenet - Camera ]
    REGISTER_API(Camera*, void, MainCamera);
    REGISTER_API(Camera*, void, CurrentCamera);

    // Event
    REGISTER_API(void, void, EventUpdate);

    // AssetManager
    REGISTER_FUNC("LoadTexture", Resources::LoadTextureL);
    REGISTER_FUNC("LoadShader", Resources::LoadShaderL);

    //Scene
    REGISTER_API_FROM_NAME("GetSceneGameObject", std::list<GameObject *> &, void, Scene::GetGameObject);
}
