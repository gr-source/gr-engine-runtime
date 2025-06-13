#include "scriptManager.hpp"

#include <gr-base/script/meshCollider.hpp>
#include <gr-base/script/boxCollider.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/rigidbody.hpp>
#include <gr-base/script/animator.hpp>
#include <gr-base/script/camera.hpp>

#include <gr-base/core/geometry/batching.hpp>

#include <gr-base/core/assetsManager.hpp>
#include <gr-base/core/scene.hpp>
#include <gr-base/core/event.hpp>

#include <gr-base/ecs/gameObject.hpp>
#include <gr-base/ecs/entity.hpp>

#include <gr-base/core/file/file.hpp>

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

    // First function
    m_script->registerFunction("Initialize",   reinterpret_cast<void **>(&InitializeScript));
    m_script->registerFunction("Destroy",      reinterpret_cast<void **>(&Destroy));

}
#pragma GCC diagnostic pop

