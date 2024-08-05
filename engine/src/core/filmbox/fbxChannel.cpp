#include "fbxChannel.hpp"

FbxChannel::FbxChannel(const std::string &name) : m_name(name) {}

const std::string &FbxChannel::getName() const {
    return m_name;
}

const std::vector<FbxKey<Vector3>> FbxChannel::getPosition() const {
    return m_position;
}

const std::vector<FbxKey<Vector3>> FbxChannel::getScale() const {
    return m_scale;
}

const std::vector<FbxKey<Quaternion>> FbxChannel::getRotation() const {
    return m_rotation;
}
