#include "keyframe.h"

Keyframe::Keyframe(const std::string& name) : name(name) {}

Matrix4x4 Keyframe::getlocalmatrix(float time) {
    return Math::translate(getInterpolatePosition(time)) * Math::rotate(getInterpolateRotation(time)) * Math::scale(getInterpolateScaling(time));
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

Quaternion nlerp(Quaternion a, Quaternion b, float blend) {
	//cout << a.w + a.x + a.y + a.z << endl;
	a = Math::normalize(a);
	b = Math::normalize(b);

	Quaternion result;
	float dot_product = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	float one_minus_blend = 1.0f - blend;

	if (dot_product < 0.0f)
	{
		result.x = a.x * one_minus_blend + blend * -b.x;
		result.y = a.y * one_minus_blend + blend * -b.y;
		result.z = a.z * one_minus_blend + blend * -b.z;
		result.w = a.w * one_minus_blend + blend * -b.w;
	}
	else
	{
		result.x = a.x * one_minus_blend + blend * b.x;
		result.y = a.y * one_minus_blend + blend * b.y;
		result.z = a.z * one_minus_blend + blend * b.z;
		result.w = a.w * one_minus_blend + blend * b.w;
	}

	return Math::normalize(result);
}

Quaternion Keyframe::getInterpolateRotation(float time) {
    if (1 == keyRotation.size()) {
        return keyRotation[0].value;
    }

    int p0Index = getIndexFromTime<Quaternion>(time, keyRotation);
    int p1Index = p0Index + 1;

    float scaleFactor = getScaleFactor(keyRotation[p0Index].time, keyRotation[p1Index].time, time);
    
    // glm::quat finalRotation = glm::slerp(
    //     glm::quat(
    //         keyRotation[p0Index].value.w,
    //         keyRotation[p0Index].value.x,
    //         keyRotation[p0Index].value.y,
    //         keyRotation[p0Index].value.z
    //     ),
    //     glm::quat(
    //         keyRotation[p1Index].value.w,
    //         keyRotation[p1Index].value.x,
    //         keyRotation[p1Index].value.y,
    //         keyRotation[p1Index].value.z
    //     ),
    //     scaleFactor
    // );

    // finalRotation = glm::normalize(finalRotation);

    // return /*nlerp(keyRotation[p0Index].value, keyRotation[p1Index].value, scaleFactor);*/Quaternion(
    //     finalRotation.w,
    //     finalRotation.x,
    //     finalRotation.y,
    //     finalRotation.z
    // );
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

float Keyframe::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    return (midWayLength / framesDiff);
}

void Keyframe::addKeyPosition(const Vector3& position, float time) {
    keyPosition.emplace_back(position, time);
}

void Keyframe::addKeyRotation(const Quaternion& rotation, float time) {
    keyRotation.emplace_back(rotation, time);
}

void Keyframe::addKeyScale(const Vector3& scale, float time) {
    keyScale.emplace_back(scale, time);
}

const std::string &Keyframe::getname() const {
    return name;
}
