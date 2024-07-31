#pragma once

#include "../ecs/gameObject.h"

#include "../component/default.hpp"

#include "../console.hpp"

#include <string>

class ScriptManager {
public:
    static void Initialzie();

    static void Shutdown();

    // Main
    static std::unordered_map<int, std::pair<const std::string, const std::string>> (*GetShader)();

    // [ Componenet - Camera ]
    static Camera *(*MainCamera)();
    static Camera *(*CurrentCamera)();

    static std::vector<GameObject*> &(*GetGameObjectsFromRender)();

    static std::vector<std::unique_ptr<GameObject>> &(*GetAllGameObject)();

    // [ Entity ]
    static std::size_t (*GetEntityCount)();

    static void (*SetDeltaTime)(float);

    // Light
    static std::vector<Light *> &(*GetListLight)();

    static void (*Update)();

    // Event;
    static void (*EventUpdate)();

    // AssetManager
    // grr::gTexture *Resources::LoadTexture(const std::string &path)
    
private:
    static void RegisterApi();

protected:
    static void *s_handle;

    // [ Main ]
    static void (*Initialize)();
    

    static void (*Destroy)();
};


