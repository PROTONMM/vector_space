#ifndef PARTICLE_H
#define PARTICLE_H

#include <SDL2/SDL.h>

#include "vector.h"

// Short-lived effect particle used for explosions and engine effects.
class Particle {
public:
    Vec2 pos;
    Vec2 vel;
    float life = 0.0f;
    float maxLife = 1.0f;
    bool alive = true;

    void Update(float dt);
    void Draw(SDL_Renderer* renderer) const;
};

#endif // PARTICLE_H
