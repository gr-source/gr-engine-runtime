#pragma once

#include "../filmbox/BoneInfo.h"
#include "../Global.h"

#include "bone.h"

enum HumanBodyBones {
    Head
};

class Animation {
public:
    std::string name;

    float duration;

    float ticksPerSecound;

    //std::vector<Bone> pBones;
    
    AssimpNodeData m_RootNode;
    
    std::map<std::string, BoneInfo> m_BoneInfoMap;

    Animation(const std::string& name);

   // Bone* findBone(const std::string& name);
};


