#include "animationClip.h"

#include <algorithm>

AnimationClip::AnimationClip(const std::string& name) : name(name), duration(0.0f), ticks(0.0f) {}

void AnimationClip::setduration(float val) {
    duration = val;
}

void AnimationClip::setmaxtick(float val) {
    ticks = val;
}

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
