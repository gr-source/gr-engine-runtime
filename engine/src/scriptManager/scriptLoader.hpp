#pragma once

#include <vector>
#include <string>

using Script = void *;

class ScriptLoader {
public:
    ScriptLoader(const std::string &path);
    ~ScriptLoader();

    bool open(const std::string &path);

    bool isValid() const;

    void replaceLibraryFunction(const std::string &name, void *ptr);
    
    void replaceLibraryField(const std::string &name, void *ptr);

    void registerFunction(const std::string &name, void **ptr);

    void close();

private:
    std::vector<void **> m_funcMap;
    
    Script m_script;

    Script FindSymbol(const std::string &name);

};
