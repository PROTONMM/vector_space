#include "pirate.h"

#include <cstdlib>

#include "vector.h"

void Pirate::Spawn(bool enterFromLeft, bool isSmall, int level) {
    small = isSmall;
    radius = small ? 15.0f : 25.0f;
    pos.x = enterFromLeft ? -radius : kCanvasWidth + radius;
    pos.y = 130.0f + (float)(std::rand() % (kCanvasHeight - 260));
    const float speed = (small ? 92.0f : 72.0f) + level * 2.0f;
    vel = {enterFromLeft ? speed : -speed, 0.0f};
    fireTimer = small ? 0.8f : 1.25f;
    courseTimer = 0.7f;
    active = true;
}

void Pirate::Update(float dt) {
    if (!active) return;
    courseTimer -= dt;
    if (courseTimer <= 0.0f) {
        // The saucer changes altitude in discrete steps, like an arcade enemy.
        vel.y = (float)((std::rand() % 3) - 1) * (small ? 48.0f : 34.0f);
        courseTimer = 0.55f + (std::rand() % 80) / 100.0f;
    }
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    if (pos.y < 105.0f) { pos.y = 105.0f; vel.y = -vel.y; }
    if (pos.y > kCanvasHeight - 90.0f) { pos.y = kCanvasHeight - 90.0f; vel.y = -vel.y; }
    if (pos.x < -radius * 2.0f || pos.x > kCanvasWidth + radius * 2.0f)
        active = false;
    fireTimer -= dt;
}

void Pirate::Draw(SDL_Renderer* renderer) const {
    if (!active) return;
    const int x = (int)pos.x;
    const int y = (int)pos.y;
    const int r = (int)radius;
    SDL_SetRenderDrawColor(renderer, 0xaa, 0xff, 0xbb, 255);
    SDL_RenderDrawLine(renderer, x-r, y, x-r/2, y-r/3);
    SDL_RenderDrawLine(renderer, x-r/2, y-r/3, x+r/2, y-r/3);
    SDL_RenderDrawLine(renderer, x+r/2, y-r/3, x+r, y);
    SDL_RenderDrawLine(renderer, x+r, y, x+r/2, y+r/3);
    SDL_RenderDrawLine(renderer, x+r/2, y+r/3, x-r/2, y+r/3);
    SDL_RenderDrawLine(renderer, x-r/2, y+r/3, x-r, y);
    SDL_RenderDrawLine(renderer, x-r, y, x+r, y);
    SDL_RenderDrawLine(renderer, x-r/3, y-r/3, x-r/5, y-r*2/3);
    SDL_RenderDrawLine(renderer, x-r/5, y-r*2/3, x+r/5, y-r*2/3);
    SDL_RenderDrawLine(renderer, x+r/5, y-r*2/3, x+r/3, y-r/3);
}
