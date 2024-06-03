#pragma once

#include <string>
#include <vector>

#include "keyframe.h"

#include "../types.h"

class AnimationClip {
public:

    AnimationClip(const std::string& name = "default");

    void setduration(float val);

    const float& getduration() const;

    void setticks(float val);

    const float& getticks() const;

    Keyframe* findKeyframe(const std::string& name);

    template <typename... Args>
    Keyframe& addkeyframe(Args&&... args);

    const std::vector<Keyframe>& getKeyList() const;

    const ptrsize getKeyframecout() const;

    const std::string& getname() const;

protected:
    std::vector<Keyframe> m_keyframes;

    std::string m_name;

    float m_duration;

    float m_ticks;
};

template <typename... Args>
inline Keyframe &AnimationClip::addkeyframe(Args &&...args) {
    return m_keyframes.emplace_back(std::forward<Args>(args)...);
}
