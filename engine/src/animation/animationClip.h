#pragma once

#include <string>
#include <vector>

#include "keyframe.h"

#include "../filmbox/BoneInfo.h"

class AnimationClip {
public:

    AnimationClip(const std::string& name, float duration = 0.0f, float ticks = 0.0f);

    Keyframe* findKeyframe(const std::string& name);

    template <typename... Args>
    Keyframe& addkeyframe(Args&&... args);

    const std::string& getname() const;

    const float& getduration() const;

    const float& getticks() const;
protected:
    std::vector<Keyframe> keyframes;

    std::string name;

    float duration;

    float ticks;
};

template <typename... Args>
inline Keyframe &AnimationClip::addkeyframe(Args &&...args) {
    return keyframes.emplace_back(std::forward<Args>(args)...);
}
