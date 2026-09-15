#ifndef PIRATE_H
#define PIRATE_H

#include <SDL2/SDL.h>

#include "vector.h"

class Pirate {
public:
    Vec2 pos;
    Vec2 vel;
    bool active = false;
    bool small = false;
    float radius = 24.0f;
    float fireTimer = 0.0f;
    float courseTimer = 0.0f;

    void Spawn(bool enterFromLeft, bool isSmall, int level);
    void Update(float dt);
    void Draw(SDL_Renderer* renderer) const;
};

#endif
