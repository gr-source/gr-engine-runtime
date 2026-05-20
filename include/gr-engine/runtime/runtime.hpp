#pragma once

#include "gr-base/managers/octreemanager.hpp"
#include "gr-base/managers/bvhtreemanager.hpp"
#include "gr-base/managers/sceneManager.hpp"

class Runtime
{
public:
    std::unique_ptr<OctreeManager> s_OctreeManager;

    std::unique_ptr<BVHTreeManager> s_BVHTreeManager;

    std::unique_ptr<SceneManager> s_SceneManager;

    inline Runtime() :
        s_OctreeManager(std::make_unique<OctreeManager>()),
        s_BVHTreeManager(std::make_unique<BVHTreeManager>()),
        s_SceneManager(std::make_unique<SceneManager>())
    {}

    virtual ~Runtime() {}

    virtual void initialize() = 0;
};


