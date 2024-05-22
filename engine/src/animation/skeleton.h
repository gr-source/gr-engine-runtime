#pragma once

#include <vector>
#include <algorithm>

#include "bone.h"

#include "../types.h"

class Skeleton {
public:
    Skeleton() = default;

    Skeleton(const Skeleton& rhs);

    Bone* findBone(const std::string& name);

    Bone& getroot();

    const bool hasbone() const;
protected:
    Bone m_root;

    friend class Filmbox;
};



