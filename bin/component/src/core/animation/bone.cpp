#include "core/animation/bone.hpp"

#include "core/stream.hpp"

Bone::Bone(const std::string & name, const Vector3 & position, const Vector3 & scale, const Quaternion & rotation, int boneIndex) : m_name(name), m_position(position), m_scale(scale), m_rotation(rotation), m_boneIndex(boneIndex), m_localMatrix(Matrix4x4::identityMatrix) {}

std::string Bone::getName() const {
    return m_name;
}

bool Bone::serizalize(Stream *stream) const {
    auto nsize = m_name.size();

    if (!stream->write(&nsize, sizeof(std::size_t))) {
        return false;
    }

    if (!stream->write(m_name.data(), nsize * sizeof(char))) {
        return false;
    }

    if (!stream->write(&m_position, sizeof(Vector3))) {
        return false;
    }

    if (!stream->write(&m_scale, sizeof(Vector3))) {
        return false;
    }

    if (!stream->write(&m_rotation, sizeof(Quaternion))) {
        return false;
    }

    if (!stream->write(&m_boneIndex, sizeof(grm::u32))) {
        return false;
    }

    return true;
}

bool Bone::deserizalize(Stream *stream) {
    std::size_t nsize = 0;
    if (!stream->read(&nsize, sizeof(std::size_t))) {
        return false;
    }

    m_name.resize(nsize, '\0');
    if (!stream->read(m_name.data(), nsize * sizeof(char))) {
        return false;
    }

    if (!stream->read(&m_position, sizeof(Vector3))) {
        return false;
    }

    if (!stream->read(&m_scale, sizeof(Vector3))) {
        return false;
    }

    if (!stream->read(&m_rotation, sizeof(Quaternion))) {
        return false;
    }

    if (!stream->read(&m_boneIndex, sizeof(grm::u32))) {
        return false;
    }

    return true;
}


