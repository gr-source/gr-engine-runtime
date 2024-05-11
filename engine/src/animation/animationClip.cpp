#include "animationClip.h"

#include <algorithm>

AnimationClip::AnimationClip(const std::string& name, float duration, float ticks) : name(name), duration(duration), ticks(ticks) {}

Keyframe* AnimationClip::findKeyframe(const std::string& name) {
    auto result = std::find_if(keyframes.begin(), keyframes.end(), [=](auto& key) {
        return key.getname() == name;
    });
    if (result != keyframes.end())
        return &(*result);
    return nullptr;
}

const std::string &AnimationClip::getname() const {
    return name;
}

const float &AnimationClip::getduration() const {
    return duration;
}

const float &AnimationClip::getticks() const {
    return ticks;
}
