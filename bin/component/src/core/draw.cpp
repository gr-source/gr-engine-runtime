#include "core/draw.hpp"
#include "core/function.hpp"

void (*Draw::CallInternalDrawLine)(const Vector3 &start, const Vector3 &end, const Color &color);
INSERT_FUNCTION(0, Draw::CallInternalDrawLine)

void Draw::Line(const Vector3 &start, const Vector3 &end, const Color &color) {
    return CallInternalDrawLine(start, end, color);
}