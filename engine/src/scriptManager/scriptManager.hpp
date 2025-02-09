#pragma once

#include "script.hpp"

#include <gr-base/ecs/componentPool.hpp>

#include "default.hpp"

#include <gr-base/core/console.hpp>

#include <gr-render/gShader.h>

class ScriptManager {
public:
    static void Initialize(const std::string &path);

    static void Shutdown();

    static bool IsValid();

    static void (*Update)();

    // Component Pool
    static bool CheckScriptLoaded(const std::string &name);
    
private:
    static void RegisterApi();

protected:
    static grm::size s_lastTime;

    static std::string s_path;

    static bool s_bReload;

    static std::unique_ptr<Script> m_script;

    // [ Main ]
    static void (*InitializeScript)();

    static void (*Destroy)();
};


