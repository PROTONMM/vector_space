#ifndef BULLET_H
#define BULLET_H

#include <SDL2/SDL.h>

#include "vector.h"

// Projectile that wraps around the playfield and expires after a short time.
class Bullet {
public:
    Vec2 pos;
    Vec2 vel;
    float life = 0.0f;
    bool alive = true;
    bool hostile = false;

    void Update(float dt);
    void Wrap();
    void Draw(SDL_Renderer* renderer) const;
};

#endif // BULLET_H
