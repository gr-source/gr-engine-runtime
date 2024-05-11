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

    bool hasbone() const;

    u32 getcount() const;
protected:
    Bone m_root;

    u32 m_count;

    friend class Filmbox;
};



