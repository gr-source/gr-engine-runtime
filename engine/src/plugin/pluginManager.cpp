#include "pluginManager.hpp"

#include <core/file/file.hpp>

#include <stdexcept>

#include <cstring>
#include <dlfcn.h>

std::vector<Plugin> PluginManager::pluginList;

void PluginManager::Load(const std::string &path) {
    if (!File::Exists(path)) {
        throw std::runtime_error("plugin do not exists.");
    }

    auto handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        throw std::runtime_error(strerror(errno));
    }
    
    Plugin plugin(handle);
    
    plugin.getPluginInfo = ((PluginInfo (*)(void)) dlsym(handle, "CreatePluginInfo"));
    if (!plugin.getPluginInfo) {
        dlclose(handle);
        throw std::runtime_error("Fail to link plugininfo.");
    }

    plugin.initialize = ((void(*)(void)) dlsym(handle, "Initialize"));
    if (!plugin.initialize) {
        dlclose(handle);
        throw std::runtime_error("Fail to link initialize.");
    }

    plugin.renderGUI = ((void(*)(void)) dlsym(handle, "Render"));
    if (!plugin.renderGUI) {
        dlclose(handle);
        throw std::runtime_error("Fail to link render.");
    }
    
    plugin.shutdown = ((void(*)(void)) dlsym(handle, "Shutdown"));
    if (!plugin.shutdown) {
        dlclose(handle);
        throw std::runtime_error("Fail to link shutdown.");
    }
    plugin.initialize();
    
    pluginList.push_back(plugin);
}

std::vector<Plugin> &PluginManager::getPluginList() {
    return pluginList;
}

void PluginManager::Shutdown() {
    for (auto &plugin : pluginList) {
        plugin.shutdown();

        dlclose(plugin.getPointer());
    }
    pluginList.clear();
}
