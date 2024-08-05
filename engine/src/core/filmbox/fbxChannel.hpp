#pragma once

#include <gr-math/gmath.h>

#include <string>
#include <vector>

template <typename T>
struct FbxKey {
    float time;

    T value;

    FbxKey(float time, T value);
};

template <typename T>
inline FbxKey<T>::FbxKey(float time, T value) : time(time), value(value) {}

class FbxChannel {
public:
    FbxChannel(const std::string &name);

    const std::string &getName() const;

    template <typename ...Args>
    FbxKey<Vector3> &addPosition(Args &&... args);

    const std::vector<FbxKey<Vector3>> getPosition() const;

    template <typename ...Args>
    FbxKey<Vector3> &addScale(Args &&... args);

    const std::vector<FbxKey<Vector3>> getScale() const;

    template <typename ...Args>
    FbxKey<Quaternion> &addRotation(Args &&... args);

    const std::vector<FbxKey<Quaternion>> getRotation() const;

private:
    std::string m_name;

    std::vector<FbxKey<Vector3>> m_position;

    std::vector<FbxKey<Vector3>> m_scale;

    std::vector<FbxKey<Quaternion>> m_rotation;
};

template <typename... Args>
inline FbxKey<Vector3> &FbxChannel::addPosition(Args &&...args) {
    return m_position.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxKey<Vector3> &FbxChannel::addScale(Args &&...args) {
    return m_scale.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxKey<Quaternion> &FbxChannel::addRotation(Args &&...args) {
    return m_rotation.emplace_back(std::forward<Args>(args)...);
}
