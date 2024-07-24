#include "keyframe.h"

Keyframe::Keyframe(const std::string& name) : name(name) {}

Matrix4x4 Keyframe::getlocalmatrix(float time) {
    return Math::translate<Matrix4x4>(getInterpolatePosition(time)) * Math::rotate<Matrix4x4>(getInterpolateRotation(time)) * Math::scale<Matrix4x4>(getInterpolateScaling(time));
}

Vector3 Keyframe::getInterpolateScaling(float time) {
    if (1 == keyScale.size())
        return keyScale[0].value;

    int p0Index = getIndexFromTime<Vector3>(time, keyScale);
    int p1Index = p0Index + 1;

    float scaleFactor = getScaleFactor(keyScale[p0Index].time, keyScale[p1Index].time, time);

    const Vector3 finalScale = Math::mix(keyScale[p0Index].value, keyScale[p1Index].value, scaleFactor);

    return finalScale;
}

Quaternion Keyframe::getInterpolateRotation(float time) {
    if (1 == keyRotation.size()) {
        return keyRotation[0].value;
    }

    int p0Index = getIndexFromTime<Quaternion>(time, keyRotation);
    int p1Index = p0Index + 1;

    float scaleFactor = getScaleFactor(keyRotation[p0Index].time, keyRotation[p1Index].time, time);
    
    return Math::slerp(keyRotation[p0Index].value, keyRotation[p1Index].value, scaleFactor);
}

Vector3 Keyframe::getInterpolatePosition(float time) {
    if (1 == keyPosition.size())
        return keyPosition[0].value;

    int p0Index = getIndexFromTime<Vector3>(time, keyPosition);
    int p1Index = p0Index + 1;

    float scaleFactor = getScaleFactor(keyPosition[p0Index].time, keyPosition[p1Index].time, time);
    
    const Vector3 finalPosition = Math::mix(
        keyPosition[p0Index].value,
        keyPosition[p1Index].value,
        scaleFactor
    );

    return finalPosition;
}

float Keyframe::getScaleFactor(float last, float next, float time) {
    float midWayLength = time - last;
    float framesDiff = next - last;
    return (midWayLength / framesDiff);
}

void Keyframe::addKeyPosition(const Vector3& position, float time) {
    keyPosition.push_back({time, position});
}

void Keyframe::addKeyRotation(const Quaternion& rotation, float time) {
    keyRotation.push_back({time, rotation});
}

void Keyframe::addKeyScale(const Vector3& scale, float time) {
    keyScale.push_back({time, scale});
}

const std::string &Keyframe::getname() const {
    return name;
}
