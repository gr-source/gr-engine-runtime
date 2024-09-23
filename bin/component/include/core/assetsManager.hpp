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

    static grr::gShader *LoadShader(const std::string &name, const std::string &frag, const std::string &vert);

    static void AddPostLoader(std::function<bool(void *, const std::string &)> func);

    static std::vector<Material> *FindMaterial(const std::string &name);

    static Mesh *FindMesh(const std::string &name);

    static grr::gShader *FindShader(const std::string &name);

private:
    static void SetGameObjectList(const std::string &path, std::function<void(GameObject *)> &func);
};

template <typename... Args>
inline void AssetsManager::LoadGameObject(const std::string &path, Args &&...args) {
    std::function<void(GameObject *)> func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);

    SetGameObjectList(path, func);
}

