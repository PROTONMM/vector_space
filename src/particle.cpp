#include "particle.h"

#include "game.h"

void Particle::Update(float dt) {
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    vel.x *= 0.96f;
    vel.y *= 0.96f;
    life -= dt;
    if (life <= 0.0f) alive = false;
}

void Particle::Draw(SDL_Renderer* renderer) const {
    float t = life / maxLife;
    uint8_t r = (uint8_t)(0x99 * t);
    uint8_t g = (uint8_t)(0xff * t);
    uint8_t b = (uint8_t)(0x66 * t);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    float size = 2.0f * t + 0.5f;
    SDL_Rect rect = { (int)(pos.x - size * 0.5f), (int)(pos.y - size * 0.5f),
                      (int)size, (int)size };
    SDL_RenderFillRect(renderer, &rect);
}
