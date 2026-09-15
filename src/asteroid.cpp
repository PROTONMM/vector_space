#include "asteroid.h"

#include <cmath>
#include <cstdlib>

#include "game.h"

float Asteroid::RadiusForSize(int size) {
    switch (size) {
        case Large: return 42.0f;
        case Medium: return 26.0f;
        case Small: return 14.0f;
    }
    return 20.0f;
}

void Asteroid::Reset(float x, float y, int size, uint32_t seed) {
    this->size = size;
    radius = RadiusForSize(size);
    pos.x = x;
    pos.y = y;

    // Random velocity; smaller rocks move faster.
    float speedScale = (size == Large) ? 30.0f : (size == Medium) ? 50.0f : 75.0f;
    float a = (seed * 2654435761u) % 1000 / 1000.0f * 6.2831853f;
    float sp = (seed % 100) / 100.0f * speedScale + 8.0f;
    vel.x = std::cos(a) * sp;
    vel.y = std::sin(a) * sp;

    angle = (seed % 1000) / 1000.0f * 6.2831853f;
    spin = ((seed % 50) / 50.0f - 0.5f) * 3.0f;

    // Generate an irregular polygon.
    int n = 10 + (size == Large ? 4 : (size == Medium ? 2 : 0));
    verts.clear();
    for (int i = 0; i < n; ++i) {
        float wob = 0.75f + ((seed + (uint32_t)i * 7919u) % 100) / 100.0f * 0.5f;
        Vec2 v;
        float ang = (float)i / n * 6.2831853f;
        v.x = std::cos(ang) * wob;
        v.y = std::sin(ang) * wob;
        verts.push_back(v);
    }
    alive = true;
}

void Asteroid::Update(float dt) {
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    angle += spin * dt;
    Wrap();
}

void Asteroid::Wrap() {
    if (pos.x < 0.0f)  pos.x += kCanvasWidth;
    else if (pos.x >= kCanvasWidth) pos.x -= kCanvasWidth;
    if (pos.y < 0.0f)  pos.y += kCanvasHeight;
    else if (pos.y >= kCanvasHeight) pos.y -= kCanvasHeight;
}

void Asteroid::Draw(SDL_Renderer* renderer) const {
    const Color col{0x99, 0xff, 0x99};
    std::vector<SDL_Point> sdlPts(verts.size());
    for (size_t i = 0; i < verts.size(); ++i) {
        float vx = std::cos(angle) * verts[i].x * radius
                 - std::sin(angle) * verts[i].y * radius;
        float vy = std::sin(angle) * verts[i].x * radius
                 + std::cos(angle) * verts[i].y * radius;
        sdlPts[i].x = (int)(pos.x + vx);
        sdlPts[i].y = (int)(pos.y + vy);
    }
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
    for (size_t i = 0; i < verts.size(); ++i) {
        int j = (i + 1) % verts.size();
        SDL_RenderDrawLine(renderer, sdlPts[i].x, sdlPts[i].y,
                           sdlPts[j].x, sdlPts[j].y);
    }
}
