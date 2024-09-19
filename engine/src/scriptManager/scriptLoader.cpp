#include "scriptLoader.hpp"

#include "core/file/file.hpp"
#include "console.hpp"

#include "../memory.h"

#include <dlfcn.h>

std::vector<void **> ScriptLoader::m_funcMap;

Script ScriptLoader::m_script = nullptr;

bool ScriptLoader::open(const std::string &path) {
    if (!File::Exists(path)) {
        Console::log(LogInfo::ERROR, "Falied to open script file do not found: " + path);
        return false;
    }

    m_script = dlopen(path.c_str(), RTLD_LAZY);
    if (!m_script) {
        Console::log(LogInfo::ERROR, "Falied to load script: " + std::string(dlerror()));
        return false;
    }

    Console::log(LogInfo::INFO, "Script laoded sucess full.");

    return true;
}

void ScriptLoader::replaceLibraryFunction(const std::string &name, void *ptr) {
    auto symbol = FindSymbol(name);
    if (!symbol) {
        return;
    }
    Memory::Hook(symbol, ptr);
}

void ScriptLoader::replaceLibraryField(const std::string &name, void *ptr) {
    auto symbol = FindSymbol(name);
    if (!symbol) {
        return;
    }
    symbol = ptr;
}

void ScriptLoader::registerFunction(const std::string &name, void **ptr) {
    auto symbol = FindSymbol(name);
    if (!symbol) {
        return;
    }
    *m_funcMap.emplace_back(ptr) = symbol;
}

void ScriptLoader::close() {
    if (!m_script) {
        return;
    }
    for (auto &ptr : m_funcMap) {
        *ptr = nullptr;
    }
    m_funcMap.clear();

    if (dlclose(m_script) != 0) {
        Console::log(LogInfo::ERROR, "Falied to close script: " + std::string(dlerror()));
    }
    m_script = nullptr;
}

Script ScriptLoader::FindSymbol(const std::string &name) {
    auto ptr = dlsym(m_script, name.c_str());
    if (!ptr) {
        throw std::runtime_error("Symbol do not found: " + name);
    }
    return ptr;
}
