#pragma once

#include "plugin.hpp"

#include <string>
#include <vector>

class PluginManager {
public:
    static void Load(const std::string &path);
    
    static std::vector<Plugin> &getPluginList(); 

    static void Shutdown();

private:

    static std::vector<Plugin> pluginList; 
};