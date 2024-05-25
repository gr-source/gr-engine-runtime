#pragma once

#include <gr-math/gmath.h>

#include <vector>

template <typename T>
struct Key {
    float time;

    T value;
};

class Keyframe {
public:
    Keyframe(const std::string& name);

    Matrix4x4 getlocalmatrix(float time);

    Vector3 getInterpolateScaling(float time);

    Quaternion getInterpolateRotation(float time);

    Vector3 getInterpolatePosition(float time);

    void addKeyPosition(const Vector3& position, float time);

    void addKeyRotation(const Quaternion& rotation, float time);

    void addKeyScale(const Vector3& scale, float time);

    const std::string& getname() const;

protected:
    template <typename T>
    std::uint32_t getIndexFromTime(float time, const std::vector<Key<T>>& keys);

    float getScaleFactor(float last, float next, float time);

    std::vector<Key<Vector3>> keyPosition;

    std::vector<Key<Quaternion>> keyRotation;
    
    std::vector<Key<Vector3>> keyScale;

    std::string name;
};

template <typename T>
inline std::uint32_t Keyframe::getIndexFromTime(float time, const std::vector<Key<T>>& keys) {
    for (std::size_t i=0;i<keys.size() - 1;i++) {
        if (time < keys[i + 1].time)
            return i;
    }
    return 0;
}
