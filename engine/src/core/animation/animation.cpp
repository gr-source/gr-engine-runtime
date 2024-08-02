#include "animation.hpp"

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
