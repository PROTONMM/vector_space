#include "bullet.h"

#include "game.h"

void Bullet::Update(float dt) {
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    life -= dt;
    if (life <= 0.0f) alive = false;
    Wrap();
}

void Bullet::Wrap() {
    if (pos.x < 0.0f)  pos.x += kCanvasWidth;
    else if (pos.x >= kCanvasWidth) pos.x -= kCanvasWidth;
    if (pos.y < 0.0f)  pos.y += kCanvasHeight;
    else if (pos.y >= kCanvasHeight) pos.y -= kCanvasHeight;
}

void Bullet::Draw(SDL_Renderer* renderer) const {
    const Color col = hostile ? Color{0xff, 0xaa, 0x88} : Color{0xcc, 0xff, 0xcc};
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
    SDL_Rect dot = { (int)pos.x - 1, (int)pos.y - 1, 2, 2 };
    SDL_RenderFillRect(renderer, &dot);
}
