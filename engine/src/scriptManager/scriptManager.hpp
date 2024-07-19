#pragma once

#include <lua5.4/lua.h>
#include <lua5.4/lauxlib.h>
#include <lua5.4/lualib.h>

#include <string>

class ScriptManager {
public:
    static void Initialize();

    static const bool Execute(const std::string& path);

    static const bool IsFunction();

    static void CallFunction(int argc, int result);

    static const bool GetField(const std::string& name);

    static void CreateMetatable(const std::string& name);

    static void GetMetatable(const std::string& name);

    static void SetMetatable(int index);

    static void PushString(const std::string& name);

    static void CreateTable();

    template <typename Callback>
    static void CallingMethod(const std::string& name, Callback&& callback);

    static void SetTable(int index);

    static const bool GetGlobal(const std::string& name);

    static void SetGloabal(const std::string& name);

    static std::string GetString();

    static void Clean();

    static const bool IsTable();

    static void Destroy();

    static lua_State *GetState();

private:
    static lua_State *state;
};

template <typename Callback>
inline void ScriptManager::CallingMethod(const std::string &name, Callback &&callback) {
    lua_pushstring(state, name.c_str());
    lua_pushcfunction(state, std::forward<Callback>(callback));
    lua_settable(state, -3);
}
