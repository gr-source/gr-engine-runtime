#include "core/raycast.hpp"
#include "core/function.hpp"

bool (*Raycast::RayCast)(const Vector3 &rayStart, const Vector3 &rayEnd);
INSERT_FUNCTION(0, Raycast::RayCast)



