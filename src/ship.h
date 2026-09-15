#ifndef SHIP_H
#define SHIP_H

#include <SDL2/SDL.h>

#include "vector.h"

// Player ship with inertial movement, rotation, thrust, and weapons.
class Ship {
public:
    Vec2 pos;
    Vec2 vel;
    float angle = -1.5707963f; // facing up
    float rot = 0.0f;
    float thrust = 0.0f;
    float enginePhase = 0.0f;

    bool alive = true;
    float thrustFuel = 100.0f;

    float radius = 12.0f;

    void Reset(float cx, float cy);
    void Update(float dt, bool thrusting, bool left, bool right);
    void Wrap();

    void Draw(SDL_Renderer* renderer) const;
};

#endif // SHIP_H
