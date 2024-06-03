#include "screen.h"

<<<<<<< HEAD
u32 Screen::width = 0.0f;
u32 Screen::height = 0.0f;

void Screen::SetResolution(u32 w, u32 h) {
=======
float Screen::width = 0.0f;
float Screen::height = 0.0f;

void Screen::SetResolution(float w, float h) {
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
    width = w;
    height = h;
}

const Vector2 Screen::GetResolution() {
<<<<<<< HEAD
    return {(float)Screen::width, (float)Screen::height};
=======
    return {Screen::width, Screen::height};
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
}
