#pragma once

#include "../component/default.hpp"

#include "../gameObject.h"

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

    static void (*EntityProcess)();

    static void (*SetDeltaTime)(float);

    // Light
    static std::vector<Light *> &(*GetListLight)();

private:
    static void RegisterApi();

protected:
    static void *s_handle;

    // [ Main ]
    static void (*Initialize)();
    static void (*Destroy)();
};


