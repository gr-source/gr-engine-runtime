#include "screen.h"

u32 Screen::width = 0.0f;
u32 Screen::height = 0.0f;

void Screen::SetResolution(u32 w, u32 h) {
    width = w;
    height = h;
}

const Vector2 Screen::GetResolution() {
    return {(float)Screen::width, (float)Screen::height};
}
