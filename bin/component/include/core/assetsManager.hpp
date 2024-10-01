#pragma once

#include <gr-render/gRender.h>

#include <functional>

#include "script/default.hpp"

#include "geometry/material.hpp"

class AssetsManager {
public:
    template <typename T>
    static T *Load(const std::string &lhs, const std::string &rhs = "");

    template <typename ...Args>
    static void LoadGameObject(const std::string &path, Args &&...args);

    template <typename ...Args>
    static void LoadShader(const std::string &name, const std::string &frag, const std::string &vert, Args &&...args);

    static void AddPostLoader(std::function<bool(void *, const std::string &)> func);

    static std::vector<Material> *FindMaterial(const std::string &name);

    static Mesh *FindMesh(const std::string &name);

    static grr::gShader *FindShader(const std::string &name);

private:
    static void SetGameObjectList(const std::string &path, std::function<void(GameObject *)> func);

    static void SetShaderList(const std::string &name, const std::string &frag, const std::string &vert, std::function<void(grr::gShader *)> func);
};

template <typename... Args>
inline void AssetsManager::LoadGameObject(const std::string &path, Args &&...args) {
    std::function<void(GameObject *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    SetGameObjectList(path, func);
}

template <typename... Args>
inline void AssetsManager::LoadShader(const std::string &name, const std::string &frag, const std::string &vert, Args &&...args) {
    std::function<void(grr::gShader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }
    
    SetShaderList(name, frag, vert, func);
}
