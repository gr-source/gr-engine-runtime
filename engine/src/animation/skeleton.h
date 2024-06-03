#pragma once

#include <vector>
#include <algorithm>

#include "bone.h"

#include "../types.h"

class Skeleton {
public:
    Skeleton(const std::string& name);
    ~Skeleton();

    const std::string& getname() const;

    Bone* findBone(const std::string& name);

    Bone* getroot();

    const Bone* getroot() const;

    const bool hasbone() const;
protected:
    std::string m_name;

    Bone* m_root;

    Bone* find(Bone* bone, const std::string& name);

    friend class Filmbox;
};



