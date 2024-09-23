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
    Key<Vector3> &addKeyPosition(Args &&...args);

    template <typename ...Args>
    Key<Vector3> &addKeyScale(Args &&...args);

    template <typename ...Args>
    Key<Quaternion> &addKeyRotation(Args &&...args);

private:
    std::string m_name;

    std::vector<Key<Vector3>> m_keyPosition;

    std::vector<Key<Vector3>> m_keyScale;

    std::vector<Key<Quaternion>> m_keyRotation;

};

template <typename... Args>
inline Key<Vector3> &Keyframe::addKeyPosition(Args &&...args) {
    return m_keyPosition.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline Key<Vector3> &Keyframe::addKeyScale(Args &&...args) {
    return m_keyScale.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline Key<Quaternion> &Keyframe::addKeyRotation(Args &&...args) {
    return m_keyRotation.emplace_back(std::forward<Args>(args)...);
}
