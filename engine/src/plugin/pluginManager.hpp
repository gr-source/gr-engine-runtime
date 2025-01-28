#pragma once

#include "plugin.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

class PluginManager {
public:
    static void Load(const std::string &path);
    
    static std::vector<Plugin *> getPluginList(); 

    static void Remove(Plugin *plugin);

    static void Shutdown();

private:
    static std::unordered_map<Plugin *, std::unique_ptr<Plugin>> pluginMap; 

    static std::vector<Plugin *> pluginList; 
};