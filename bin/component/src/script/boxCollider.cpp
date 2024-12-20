#include "script/boxCollider.hpp"

#include "script/transform.hpp"

#include "core/function.hpp"

#include <core/file/stream.hpp>

BoxCollider::BoxCollider() : m_CollisionShape(nullptr), boxSize(Vector3::one) {
    buildShape(boxSize);
}

bool BoxCollider::Serialize(Stream *stream) {
    stream->write(&boxSize, sizeof(Vector3));
    return true;
}

bool BoxCollider::Deserialize(Stream *stream) {
    stream->read(reinterpret_cast<char *>(&boxSize), sizeof(Vector3));
    return true;
}


static void (*BoxColliderbuildShape)(BoxCollider *, const Vector3 &);
INSERT_FUNCTION(0, BoxColliderbuildShape)

void BoxCollider::buildShape(const Vector3 &size) {
    return BoxColliderbuildShape(this, size);
}
