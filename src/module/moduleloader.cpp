#include "gr-engine/module/moduleloader.hpp"
#include "gr-base/core/logger.hpp"
#include <cstring>
#include <dlfcn.h>
#include <stdexcept>

ModuleLoader* ModuleLoader::s_ModuleLoader = nullptr;

void ModuleLoader::Initialize()
{
    if (s_ModuleLoader == nullptr)
        s_ModuleLoader = new ModuleLoader();
}

void ModuleLoader::Shutdown()
{
    if (s_ModuleLoader != nullptr)
        delete s_ModuleLoader;
    s_ModuleLoader = nullptr;
}


ModuleLoader::ModuleLoader()
{
    logger::info("[ ModuleLoader ] Initialize '%p'.", this);
}

ModuleLoader::~ModuleLoader()
{
    logger::info("[ ModuleLoader ] Shutdown '%p'.", this);
}

void ModuleLoader::load(const std::string& name, const std::string& path)
{
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (handle == nullptr)
        throw std::runtime_error(strerror(errno));

    m_moduleMap.emplace(name, handle);
}

void ModuleLoader::unload(const std::string& name)
{
    auto it = m_moduleMap.find(name);
    if (it == m_moduleMap.end())
        return;

    dlclose(it->second);

    m_moduleMap.erase(it);
}

void ModuleLoader::unload()
{
    for (auto&& [name, handle] : m_moduleMap)
        dlclose(handle);
    m_moduleMap.clear();
}

void* ModuleLoader::getModule(const std::string& name)
{
    auto it = m_moduleMap.find(name);
    if (it == m_moduleMap.end())
        return nullptr;
    return it->second;
}

void* ModuleLoader::findFunction(const std::string& module, const std::string& name)
{
    auto handle = getModule(module);
    if (handle == nullptr)
        return nullptr;

    return dlsym(handle, name.c_str());
}


