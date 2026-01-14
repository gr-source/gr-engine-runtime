#pragma once

#include "script.hpp"

#include <memory>

class ScriptManager
{
public:
    static void Load(const std::string &path);

    static void Unload();

    static bool CheckScriptLoaded(const std::string &name);

private:
    std::unique_ptr<Script> m_script;

    ScriptManager();
    ~ScriptManager();

    void load(const std::string &path);

    void unload();

    void test();

protected:
    static ScriptManager &GetInstance();
};


