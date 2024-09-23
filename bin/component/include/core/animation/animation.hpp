#pragma once

#include "keyframe.hpp"

#include <unordered_map>

class Animation {
public:
    Animation(const std::string &name = "default", float duration = 0.0f, float tick = 0.0f);

    const std::string &getName() const;

    void setName(const std::string &name);

    const float getDuration() const;

    void setDuration(float value);

    const float getTick() const;

    void setTick(float value);

    template <typename ...Args>
    Keyframe &addkeyframe(Args &&... args);

private:
    std::string m_name;

    float m_duration;

    float m_tick;

    std::unordered_map<std::string, Keyframe> m_keyframes;
};

template <typename ...Args>
inline Keyframe &Animation::addkeyframe(Args &&...args) {
    auto &&[it, added] = m_keyframes.emplace(std::forward<Args>(args)...);

    return (*it).second;
}
