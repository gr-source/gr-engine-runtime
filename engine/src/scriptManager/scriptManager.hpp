#pragma once

#include "scriptLoader.hpp"

#include "../ecs/gameObject.h"

#include "console.hpp"
#include "default.hpp"

#include "core/ecs/componentPool.hpp"

#include <gr-render/gShader.h>

class ScriptManager {
public:
    static void Initialize(const std::string &path);

    static void Shutdown();

    static bool IsValid();

    // [ Entity ]
    static std::size_t (*GetEntityCount)();

    static void (*SetDeltaTime)(float);

    // Light
    static std::vector<Light *> &(*GetListLight)();

    static void (*Update)();

    static std::unique_ptr<Animation> (*LoadAnimation)(const std::string &);
    
    static void (*SetScreenSize)(const iVector2 &);

    // Component Pool
    static std::unordered_map<std::string, std::unique_ptr<Pool>> &(*GetComponentPool)();

    static Component *(*CreateComponentWithName)(const std::string &name);

    static Component *(*CopyComponentWithName)(const std::string &name, const Component *component);
    
private:
    static void RegisterApi();

protected:
    static grm::size s_lastTime;

    static std::string s_path;

    static bool s_bReload;

    static std::unique_ptr<ScriptLoader> m_script;

    // [ Main ]
    static void (*InitializeScript)();
    

    static void (*Destroy)();
};


