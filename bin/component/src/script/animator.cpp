#include "script/animator.hpp"

#include "ecs/gameObject.hpp"

#include "core/function.hpp"
#include "core/stream.hpp"

static void (*AnimatorUpdate)(Animator *);
INSERT_FUNCTION(0, AnimatorUpdate)

Animator::Animator() : m_armature(nullptr), m_animation(nullptr), m_currentTime(0.0f), m_FinalBoneMatrices(100, Matrix4x4::identityMatrix) {}

void Animator::Update() {
    return AnimatorUpdate(this);
}

bool Animator::Serialize(Stream *stream) {
    bool result = true;

    auto bsize = m_bones.size();
    stream->write(&bsize, sizeof(std::size_t));

    for (auto &&[name, bone] : m_bones) {
        auto nsize = name.size();

        stream->write(&nsize, sizeof(std::size_t));
        stream->write(name.data(), nsize * sizeof(char));

        result |= bone->serizalize(stream);
    }
    return result;
}

bool Animator::Deserialize(Stream *stream) {
    bool result = true;

    std::size_t bsize = 0;
    stream->read(&bsize, sizeof(std::size_t));

    for (std::size_t i={};i<bsize;i++) {
        std::size_t nsize = 0;

        stream->read(&nsize, sizeof(std::size_t));

        std::string name(nsize, '\0');
        stream->read(name.data(), nsize * sizeof(char));

        auto &&[it, added] = m_bones.emplace(name, std::make_shared<Bone>());

        result |= it->second->deserizalize(stream);
    }
    return result;
}
