#pragma once

#include "gr-base/core/renderSystem.hpp"
#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/managers/gameobjectmanager.hpp"
#include "gr-base/managers/octreemanager.hpp"
#include "gr-base/managers/bvhtreemanager.hpp"
#include "gr-base/managers/rendermanager.hpp"
#include "gr-base/managers/sceneManager.hpp"

class Runtime
{
public:
    std::unique_ptr<GameObjectManager> s_GameObjectManager;

    std::unique_ptr<BehaviorStorage> s_BehaviorStorage;

    std::unique_ptr<RenderSystem> s_RenderSystem;

    std::unique_ptr<OctreeManager> s_OctreeManager;

    std::unique_ptr<BVHTreeManager> s_BVHTreeManager;

    std::unique_ptr<RenderManager> s_RenderManager;

    std::unique_ptr<SceneManager> s_SceneManager;

    inline Runtime()
        : s_GameObjectManager(std::make_unique<GameObjectManager>()),
        s_BehaviorStorage(std::make_unique<BehaviorStorage>()),
        s_OctreeManager(std::make_unique<OctreeManager>()),
        s_BVHTreeManager(std::make_unique<BVHTreeManager>()),
        s_RenderManager(std::make_unique<RenderManager>()),
        s_SceneManager(std::make_unique<SceneManager>()),
        s_RenderSystem(std::make_unique<RenderSystem>())
    {}

    virtual ~Runtime() {}

    virtual void initialize() = 0;
};


