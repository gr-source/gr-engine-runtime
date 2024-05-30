#include "animationClip.h"

#include <algorithm>

AnimationClip::AnimationClip(const std::string& name) : m_name(name), m_duration(0.0f), m_ticks(0.0f) {}

void AnimationClip::setduration(float val) {
    m_duration = val;
}

void AnimationClip::setticks(float val) {
    m_ticks = val;
}

Keyframe* AnimationClip::findKeyframe(const std::string& name) {
    auto result = std::find_if(m_keyframes.begin(), m_keyframes.end(), [=](auto& key) {
        return key.getname() == name;
    });
    if (result != m_keyframes.end())
        return &(*result);
    return nullptr;
}

const std::vector<Keyframe> &AnimationClip::getKeyList() const {
    return m_keyframes;
}

const ptrsize AnimationClip::getKeyframecout() const {
    return m_keyframes.size();
}

const std::string &AnimationClip::getname() const {
    return m_name;
}

const float &AnimationClip::getduration() const {
    return m_duration;
}

const float &AnimationClip::getticks() const {
    return m_ticks;
}
