#include "core/raycast.hpp"
#include "core/function.hpp"

bool (*Raycast::RayCast)(const Vector3 &rayStart, const Vector3 &rayEnd, Hit &hit);
INSERT_FUNCTION(0, Raycast::RayCast)



