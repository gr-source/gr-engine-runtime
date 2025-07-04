#include "screen.hpp"

int Screen::width = 0.0f;
int Screen::height = 0.0f;

void Screen::SetResolution(int w, int h) {
    width = w;
    height = h;
}

const iVector2 Screen::GetResolution() {
    return {Screen::width, Screen::height};
}
