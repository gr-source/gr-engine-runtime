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

    const std::string &getName() const;

    const Matrix4x4 getLocalMatrix(float time) const;

    const Vector3 getInterpolateScaling(float time) const;

    const Quaternion getInterpolateRotation(float time) const;

    const Vector3 getInterpolatePosition(float time) const;

    const float getScaleFactor(float last, float next, float time) const;

private:
    std::string m_name;

    std::vector<Key<Vector3>> m_keyPosition;

    std::vector<Key<Vector3>> m_keyScale;

    std::vector<Key<Quaternion>> m_keyRotation;

    template <typename T>
    const grm::u32 getIndexFromTime(float time, const std::vector<Key<T>>& keys) const;
};

template <typename T>
inline const grm::u32 Keyframe::getIndexFromTime(float time, const std::vector<Key<T>>& keys) const {
    for (auto i=0;i<keys.size() - 1;i++) {
        if (time <= keys[i + 1].time)
            return i;
    }
    return 0;
}
