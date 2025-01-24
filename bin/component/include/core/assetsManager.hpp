#pragma once

#include <gr-render/gRender.h>

#include <functional>

#include "../script/default.hpp"

#include "geometry/material.hpp"

class AssetsManager {
public:
    template <typename ...Args>
    static void LoadTexture(const std::string &path, Args &&...args);
    
    template <typename ...Args>
    static void LoadGameObject(const std::string &path, Args &&...args);

    template <typename ...Args>
    static void LoadShader(const std::string &name, const std::string &frag, const std::string &vert, Args &&...args);

    // ********** Mesh ********** //
    static Mesh *(*CreateMesh)(const std::string &key, const std::string &name);

    static Mesh *(*FindMesh)(const std::string &name);

    static grr::gShader *(*FindShader)(const std::string &name);

    static void (*SetGameObjectList)(const std::string &path, std::function<void(GameObject *)> func);

    static void (*SetShaderList)(const std::string &name, const std::string &frag, const std::string &vert, std::function<void(grr::gShader *)> func);

    static void (*SetTextureList)(const std::string &path, std::function<void(grr::gTexture *)> func);
};

template <typename... Args>
inline void AssetsManager::LoadTexture(const std::string &path, Args &&...args) {
    std::function<void(grr::gTexture *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }
    SetTextureList(path, func);
}

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
