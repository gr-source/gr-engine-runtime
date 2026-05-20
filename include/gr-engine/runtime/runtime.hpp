#pragma once

#include <memory>

#include "gr-base/managers/octreemanager.hpp"
#include "gr-base/managers/bvhtreemanager.hpp"

class Runtime
{
public:
    std::unique_ptr<OctreeManager> s_OctreeManager;

    std::unique_ptr<BVHTreeManager> s_BVHTreeManager;

    inline Runtime() :
        s_OctreeManager(std::make_unique<OctreeManager>()),
        s_BVHTreeManager(std::make_unique<BVHTreeManager>())
    {}

    virtual ~Runtime() {}

    virtual void initialize() = 0;
};


