#pragma once

#include <istream>
#include <ostream>

struct Color {
    union {
        struct {
            float r;
            float g;
            float b;
            float a;
        };
        float data[4];
    };

    Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    Color(const Color& other) : r(other.r), g(other.g), b(other.b), a(other.a) {}

    static const Color red;

    static const Color green;

    static const Color blue;

    static const Color white;

    static const Color black;

    Color& operator=(const Color& other);

    friend std::istream& operator>>(std::istream& is, Color& color) {
        is >> color.r >> color.g >> color.b >> color.a;
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const Color& color) {
        os << color.r << " " << color.g << " " << color.b << " " << color.a;
        return os;
    }
};

inline const Color Color::red = {1.0f, 0.0f, 0.0f};

inline const Color Color::green = {0.0f, 1.0f, 0.0f};

inline const Color Color::blue = {0.0f, 0.0f, 1.0f};

inline const Color Color::white = {1.0f, 1.0f, 1.0f};

inline const Color Color::black {0.0f, 0.0f, 0.0f};

inline Color& Color::operator=(const Color& other) {
    if (this != &other) {
        r = other.r;
        g = other.g;
        b = other.b;
        a = other.a;
    }
    return *this;
}

