#include "scriptManager.hpp"

#include <stdexcept>

lua_State *ScriptManager::state = nullptr;

void ScriptManager::Initialize() {
    state = luaL_newstate();
    if (!state) {
        throw std::runtime_error("Error is in create lua state.");
    }
    luaL_openlibs(state);
}

const bool ScriptManager::Execute(const std::string &path) {
    if (luaL_dofile(state, path.c_str()) != LUA_OK) {
        return false;
    }
    return true;
}

const bool ScriptManager::IsFunction() {
    return lua_isfunction(state, -1);
}

void ScriptManager::CallFunction(int argc, int result) {
    lua_pcall(state, argc, result, 0);
}

const bool ScriptManager::GetField(const std::string &name) {
    return lua_getfield(state, -1, name.c_str()) != 0;
}

void ScriptManager::CreateMetatable(const std::string &name) {
    luaL_newmetatable(state, name.c_str());
}

void ScriptManager::GetMetatable(const std::string &name) {
    // lua_getmetatable(name, -1);
}

void ScriptManager::SetMetatable(int index) {
    lua_setmetatable(state, index);
}

void ScriptManager::PushString(const std::string &name) {
    lua_pushstring(state, name.c_str());
}

void ScriptManager::CreateTable() {
    lua_newtable(state);
}

void ScriptManager::SetTable(int index) {
    lua_settable(state, index);
}

const bool ScriptManager::GetGlobal(const std::string &name) {
    if (lua_getglobal(state, name.c_str()) == LUA_OK) {
        return false;
    }
    return true;
}

void ScriptManager::SetGloabal(const std::string &name) {
    lua_setglobal(state, name.c_str());
}

std::string ScriptManager::GetString() {
    return std::string(lua_tostring(state, -1));
}

void ScriptManager::Clean() {
    lua_pop(state, 1);
}

const bool ScriptManager::IsTable() {
    return lua_istable(state, -1);
}

void ScriptManager::Destroy() {
    if (!state) {
        return;
    }

    lua_close(state);
}

lua_State *ScriptManager::GetState() {
    return state;
}
