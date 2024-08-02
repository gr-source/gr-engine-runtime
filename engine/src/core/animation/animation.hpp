#pragma once

#include "keyframe.hpp"

class Animation {
public:
    Animation(const std::string &name, float duration, float tick);

    const std::string &getName() const;

    const float getDuration() const;

    const float getTick() const;

    template <typename ...Args>
    Keyframe &addkeyframe(Args &&... args);
    
private:
    std::string m_name;

    float m_duration;

    float m_tick;

    std::vector<Keyframe> m_keyframes;
};

template <typename... Args>
inline Keyframe &Animation::addkeyframe(Args &&...args) {
    return m_keyframes.emplace_back(std::forward<Args>(args)...);
}
