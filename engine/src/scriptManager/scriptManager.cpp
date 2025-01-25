#include "scriptManager.hpp"

#include "script/meshCollider.hpp"
#include "script/boxCollider.hpp"
#include "script/meshRender.hpp"
#include "script/rigidbody.hpp"
#include "script/animator.hpp"
#include "script/camera.hpp"

#include <core/geometry/batching.hpp>

#include "core/assetsManager.hpp"
#include "core/threadpool.hpp"
#include "core/scene.hpp"
#include "core/event.hpp"
#include "core/input.hpp"

#include "physics/physicsScene.hpp"

#include "ecs/gameObject.hpp"
#include "ecs/entity.hpp"

#include "../memory.h"

#include "settings.h"

#include <iostream>
#include <fstream>
#include <dlfcn.h>

grm::size ScriptManager::s_lastTime = 0;

std::string ScriptManager::s_path = "";

bool ScriptManager::s_bReload = false;

std::unique_ptr<Script> ScriptManager::m_script = nullptr;

// [ Main ]
void (*ScriptManager::InitializeScript)();
void (*ScriptManager::Update)();
void (*ScriptManager::Destroy)();
void (*ScriptManager::SetDeltaTime)(float);

void ScriptManager::Initialize(const std::string &path) {
    if (!File::Exists(path)) {
        throw std::runtime_error("Script data do not exists: " + path);
    }
    s_path = path;

    m_script = std::make_unique<Script>(s_path);
    if (!m_script->isValid()) {
        throw std::runtime_error("Script do not is valid: " + path);
    }

    RegisterApi();

    if (InitializeScript) {
        Console::log(Log::INFO, "Script initialized.");
        InitializeScript();
    }
}

void ScriptManager::Shutdown() {
    Console::log(Log::INFO, "Script destoyed.");

    if (Destroy) {
        Destroy();
    }

    m_script = nullptr;
}

bool ScriptManager::IsValid() {
    return m_script && m_script->isValid();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"

bool ScriptManager::CheckScriptLoaded(const std::string &name) {
    std::ifstream is("/proc/self/maps");
    if (!is.is_open()) {
        return false;
    }
    bool isValid = false;
    
    std::string line;
    while (std::getline(is, line)) {
        if (line.find(name) == std::string::npos) {
            continue;
        }
        isValid = true;
        break;
    }
    is.close();
    return isValid;
}

void ScriptManager::RegisterApi() {
    // Functions Pointer
    if (!m_script->setRegistry("_ZN8Function19GetFunctionRefrenceB5cxx11Ev")) {
        return Console::LogError("Falied to register library functions.");
    }

    m_script->replaceLibraryFunction("Console::log", reinterpret_cast<void *>(&Console::log));
    m_script->replaceLibraryFunction("Threadpool::CreateInternalTask", reinterpret_cast<void *>(&Threadpool::CreateInternalTask));

    // First function
    m_script->registerFunction("Initialize",   reinterpret_cast<void **>(&InitializeScript));
    m_script->registerFunction("Destroy",      reinterpret_cast<void **>(&Destroy));

    m_script->registerFunction("SetDeltaTime", reinterpret_cast<void **>(&SetDeltaTime));
    
    // Event Manager
    m_script->replaceLibraryFunction("Event::AddInternalEvent", reinterpret_cast<void **>(&Event::AddEvent));

    // AssetManager
    m_script->replaceLibraryFunction("AssetsManager::SetTextureList", reinterpret_cast<void *>(&AssetsManager::SetTextureList));
    m_script->replaceLibraryFunction("AssetsManager::SetGameObjectList", reinterpret_cast<void *>(&AssetsManager::SetGameObjectList));
    m_script->replaceLibraryFunction("AssetsManager::FindMesh", reinterpret_cast<void *>(&AssetsManager::FindMesh));
    m_script->replaceLibraryFunction("AssetsManager::FindShader", reinterpret_cast<void *>(&AssetsManager::FindShader));
    m_script->replaceLibraryFunction("AssetsManager::SetShaderList", reinterpret_cast<void *>(&AssetsManager::SetShaderList));
    m_script->replaceLibraryFunction("AssetsManager::CreateMesh", reinterpret_cast<void *>(&AssetsManager::CreateMesh<std::string>));

    // Input
    m_script->replaceLibraryFunction("Input::IskeyPress", reinterpret_cast<void *>(&Input::IsKeyPress));
    m_script->replaceLibraryFunction("Input::SetCursorPos", reinterpret_cast<void *>(&Input::SetCursorPos));
    m_script->replaceLibraryFunction("Input::GetCursorPos", reinterpret_cast<void *>(&Input::GetCursorPos));
    m_script->replaceLibraryFunction("Input::IsMousePress", reinterpret_cast<void *>(&Input::IsMouseDown));

}
#pragma GCC diagnostic pop

