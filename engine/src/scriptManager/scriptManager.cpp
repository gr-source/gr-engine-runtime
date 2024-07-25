#include "scriptManager.hpp"

#include <iostream>

#include <dlfcn.h>

#define REGISTER_API(lhs, rhs, func) func = (lhs(*)(rhs))dlsym(ScriptManager::s_handle, #func)

#define UNREGISTER_API(func) func = nullptr

void* ScriptManager::s_handle = nullptr;

// [ Main ]
void (*ScriptManager::Initialize)();
void (*ScriptManager::Destroy)();

std::vector<std::unique_ptr<GameObject>> &(*ScriptManager::GetAllGameObject)();

// [ Entity ]
std::size_t (*ScriptManager::GetEntityCount)();

// [ Component - MeshRender ]
std::vector<GameObject*> &(*ScriptManager::GetGameObjectsFromRender)(); 

// [ Componenet - Camera ]
Camera *(*ScriptManager::MainCamera)();
Camera *(*ScriptManager::CurrentCamera)();

void ScriptManager::Initialzie() {
    s_handle = dlopen("/home/grsource/Documentos/AppProjects/gr-component/lib/libgr-component.so", RTLD_LAZY);
    if (!s_handle) {
        std::cout << "Falied to load Script." << std::endl;
        return;
    }

    RegisterApi();

    if (Initialize) {
        Initialize();
    }
}

void ScriptManager::Shutdown() {
    if (Destroy) {
        Destroy();
    }

    // [ Main ]
    UNREGISTER_API(Initialize);
    UNREGISTER_API(Destroy);

    // [ Component - MeshRender ]
    UNREGISTER_API(GetGameObjectsFromRender);
    UNREGISTER_API(GetAllGameObject);

    // [ Entity ]
    UNREGISTER_API(GetEntityCount);

    // [ Componenet - Camera ]
    UNREGISTER_API(MainCamera);
    UNREGISTER_API(CurrentCamera);
    
    if (!s_handle) {
        return;
    }

    dlclose(s_handle);

    s_handle = nullptr;
}

void ScriptManager::RegisterApi() {
    // [ Main ]
    REGISTER_API(void, void, Initialize);
    REGISTER_API(void, void, Destroy);

    // [ Component - MeshRender ]
    REGISTER_API(std::vector<GameObject*>&, void, GetGameObjectsFromRender);
    REGISTER_API(std::vector<std::unique_ptr<GameObject>>&, void, GetAllGameObject);

    // [ Entity ]
    REGISTER_API(std::size_t, void, GetEntityCount);

    // [ Componenet - Camera ]
    REGISTER_API(Camera*, void, MainCamera);
    REGISTER_API(Camera*, void, CurrentCamera);
}
