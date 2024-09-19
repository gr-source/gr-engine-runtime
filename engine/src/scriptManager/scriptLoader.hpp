#pragma once

#include <vector>
#include <string>

using Script = void *;

class ScriptLoader {
public:
    ScriptLoader() = delete;
    ~ScriptLoader() = delete;

    static bool open(const std::string &path);

    static void replaceLibraryFunction(const std::string &name, void *ptr);
    
    static void replaceLibraryField(const std::string &name, void *ptr);

    static void registerFunction(const std::string &name, void **ptr);

    static void close();

private:
    static std::vector<void **> m_funcMap;
    
    static Script m_script;

    static Script FindSymbol(const std::string &name);

};
