#pragma once

#include "../ecs/gameObject.h"

#include "console.hpp"
#include "default.hpp"

class ScriptManager {
public:
    static void Initialize();

    static void Shutdown();

    // Main
    static std::unordered_map<int, std::pair<const std::string, const std::string>> (*GetShader)();

    // [ Componenet - Camera ]
    static Camera *(*MainCamera)();
    static Camera *(*CurrentCamera)();

    static std::vector<GameObject*> &(*GetGameObjectsFromRender)();

    static std::list<std::unique_ptr<GameObject>> &(*GetAllGameObject)();

    // [ Entity ]
    static std::size_t (*GetEntityCount)();

    static void (*SetDeltaTime)(float);

    // Light
    static std::vector<Light *> &(*GetListLight)();

    static void (*Update)();

    // Event;
    static void (*EventUpdate)();

    static std::unique_ptr<Animation> (*LoadAnimation)(const std::string &);
    
    static void (*SetScreenSize)(const iVector2 &);

private:
    static void RegisterApi();

protected:
    static void *s_handle;

    // [ Main ]
    static void (*InitializeScript)();
    

    static void (*Destroy)();
};


