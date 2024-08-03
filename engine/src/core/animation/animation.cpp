#include "animation.hpp"

#include <algorithm>

Animation::Animation(const std::string &name, float duration, float tick) : m_name(name), m_duration(duration), m_tick(tick) {}

const std::string &Animation::getName() const {
    return m_name;
}

const float Animation::getDuration() const {
    return m_duration;
}

const float Animation::getTick() const {
    return m_tick;
}

Keyframe *Animation::findKeyframe(const std::string &name) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), [&name](const auto& keyframe) {
        return name == keyframe.getName();
    });
    if (it != m_keyframes.end()) {
        return &(*it);
    }
    return nullptr;
}
