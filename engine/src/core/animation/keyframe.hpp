#pragma once

#include <string>
#include <vector>

#include <gr-math/gmath.h>

template <typename T>
struct Key {
    float time;
    T value;

    Key(float time, T value);
};

template <typename T>
inline Key<T>::Key(float time, T value) : time(time), value(value) {}

class Keyframe {
public:
    Keyframe(const std::string &name);

    template <typename ...Args>
    Key<Vector3> &addPosition(Args &&...args);

    template <typename ...Args>
    Key<Vector3> &addScale(Args &&...args);

    template <typename ...Args>
    Key<Quaternion> &addRotation(Args &&...args);

private:
    std::string m_name;

    std::vector<Key<Vector3>> m_position;

    std::vector<Key<Vector3>> m_scale;

    std::vector<Key<Quaternion>> m_rotation;
};

template <typename... Args>
inline Key<Vector3> &Keyframe::addPosition(Args &&...args) {
    return m_position.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline Key<Vector3> &Keyframe::addScale(Args &&...args) {
    return m_scale.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline Key<Quaternion> &Keyframe::addRotation(Args &&...args) {
    return m_rotation.emplace_back(std::forward<Args>(args)...);
}
