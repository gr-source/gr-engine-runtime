#include "core/animation/animation.hpp"

#include <algorithm>

Animation::Animation(const std::string &name, float duration, float tick) : m_name(name), m_duration(duration), m_tick(tick) {}

const std::string &Animation::getName() const {
    return m_name;
}

void Animation::setName(const std::string &name) {
    m_name = name;
}

const float Animation::getDuration() const {
    return m_duration;
}

void Animation::setDuration(float value) {
    m_duration = value;
}

const float Animation::getTick() const {
    return m_tick;
}

void Animation::setTick(float value) {
    m_tick = value;
}
