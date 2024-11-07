#include "core/geometry/batching.hpp"

#include "core/function.hpp"

void (*Batching::InsertInternal)(Batching *instance, Transform *transform);
INSERT_FUNCTION(0, Batching::InsertInternal);

void (*Batching::RemoveInternal)(Batching *instance, Transform *transform, bool bDelete);
INSERT_FUNCTION(1, Batching::RemoveInternal);

void Batching::insert(Transform *transform) {
    return InsertInternal(this, transform);
}

void Batching::remove(Transform * transform, bool bDelete) {
    return RemoveInternal(this, transform, bDelete);
}

