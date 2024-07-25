#pragma once

#include "../component/default.hpp"

#include "../gameObject.h"

#define REGISTER_API(lhs, rhs, func) func = (lhs(*)(rhs))dlsym(ScriptManager::s_handle, #func)

class ScriptManager {
public:
    static void Initialzie();

    static void Shutdown();

    // [ Componenet - Camera ]
    static Camera *(*MainCamera)();
    static Camera *(*CurrentCamera)();

    static std::vector<GameObject*> &(*GetGameObjectsFromRender)();

    static std::vector<std::unique_ptr<GameObject>> &(*GetAllGameObject)();

    // [ Entity ]
    static std::size_t (*GetEntityCount)();

private:
    static void RegisterApi();

protected:
    static void *s_handle;

    // [ Main ]
    static void (*Initialize)();
    static void (*Destroy)();
};


