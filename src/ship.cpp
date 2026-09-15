#include "ship.h"

#include <cmath>

#include "game.h"

void Ship::Reset(float cx, float cy) {
    pos.x = cx;
    pos.y = cy;
    vel.x = 0.0f;
    vel.y = 0.0f;
    angle = -1.5707963f;
    rot = 0.0f;
    thrust = 0.0f;
    enginePhase = 0.0f;
    alive = true;
    thrustFuel = 100.0f;
}

void Ship::Update(float dt, bool thrusting, bool left, bool right) {
    const float kTurnSpeed = 4.0f;      // rad/s
    const float kThrustAccel = 320.0f;  // px/s^2
    const float kDrag = 0.12f;          // near-frictionless movement in space

    if (left)  angle -= kTurnSpeed * dt;
    if (right) angle += kTurnSpeed * dt;

    thrust = 0.0f;
    if (thrusting && alive && thrustFuel > 0.0f) {
        vel.x += std::cos(angle) * kThrustAccel * dt;
        vel.y += std::sin(angle) * kThrustAccel * dt;
        thrust = 1.0f;
        enginePhase += dt * 24.0f;
        thrustFuel = std::max(0.0f, thrustFuel - 30.0f * dt);
    } else if (!thrusting) {
        thrustFuel = std::min(100.0f, thrustFuel + 15.0f * dt);
    }

    // Slight drag makes the ship easier to control.
    float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    if (speed > 0.0f) {
        float drag = std::max(0.0f, 1.0f - kDrag * dt);
        vel.x *= drag;
        vel.y *= drag;
    }

    pos.x += vel.x * dt;
    pos.y += vel.y * dt;

    Wrap();
}

void Ship::Wrap() {
    if (pos.x < 0.0f)  pos.x += kCanvasWidth;
    else if (pos.x >= kCanvasWidth) pos.x -= kCanvasWidth;
    if (pos.y < 0.0f)  pos.y += kCanvasHeight;
    else if (pos.y >= kCanvasHeight) pos.y -= kCanvasHeight;
}

void Ship::Draw(SDL_Renderer* renderer) const {
    if (!alive) return;

    const Color col{0x66, 0xff, 0x66};
    const float r = radius;
    // Classic open-tail ship silhouette drawn as a single outline.
    const Vec2 local[5] = {
        {r * 1.45f, 0.0f}, {r * -0.90f, r * -0.78f},
        {r * -0.55f, 0.0f}, {r * -0.90f, r * 0.78f},
        {r * 1.45f, 0.0f}
    };

    SDL_Point sdlPts[5];
    for (int i = 0; i < 5; ++i) {
        const float rx = std::cos(angle) * local[i].x - std::sin(angle) * local[i].y;
        const float ry = std::sin(angle) * local[i].x + std::cos(angle) * local[i].y;
        sdlPts[i].x = (int)(pos.x + rx);
        sdlPts[i].y = (int)(pos.y + ry);
    }
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
    for (int i = 0; i < 4; ++i) {
        int j = i + 1;
        SDL_RenderDrawLine(renderer, sdlPts[i].x, sdlPts[i].y,
                           sdlPts[j].x, sdlPts[j].y);
    }

    // A pulsing symmetrical flame extends from both sides of the nozzle.
    if (thrust > 0.5f) {
        const float length = r * (1.45f + 0.28f * (0.5f + 0.5f * std::sin(enginePhase)));
        const Vec2 flameLocal[3] = {
            {r * -0.74f, r * -0.38f}, {-length, 0.0f}, {r * -0.74f, r * 0.38f}
        };
        SDL_Point flame[3];
        for (int i = 0; i < 3; ++i) {
            const float rx = std::cos(angle) * flameLocal[i].x - std::sin(angle) * flameLocal[i].y;
            const float ry = std::sin(angle) * flameLocal[i].x + std::cos(angle) * flameLocal[i].y;
            flame[i] = {(int)(pos.x + rx), (int)(pos.y + ry)};
        }
        SDL_SetRenderDrawColor(renderer, 0xcc, 0xff, 0xcc, 255);
        SDL_RenderDrawLine(renderer, flame[0].x, flame[0].y, flame[1].x, flame[1].y);
        SDL_RenderDrawLine(renderer, flame[1].x, flame[1].y, flame[2].x, flame[2].y);
    }
}
