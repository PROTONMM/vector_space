#ifndef VECTOR_H
#define VECTOR_H

#include <cstdint>

// The portrait 3:4 canvas is independent of the window size, so geometry is
// never stretched.
constexpr int kCanvasWidth = 720;
constexpr int kCanvasHeight = 960;

// Vector color styled after a green phosphor display.
struct Color {
    uint8_t r, g, b;
};

// Lightweight two-dimensional vector.
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

#endif // VECTOR_H
