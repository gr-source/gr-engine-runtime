#include "screen.h"

float Screen::width = 0.0f;
float Screen::height = 0.0f;

void Screen::SetResolution(float w, float h) {
    width = w;
    height = h;
}

const Vector2 Screen::GetResolution() {
    return {Screen::width, Screen::height};
}
