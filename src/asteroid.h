#ifndef ASTEROID_H
#define ASTEROID_H

#include <cstdint>
#include <vector>

#include <SDL2/SDL.h>

#include "vector.h"

// Irregular polygonal rock that splits into smaller fragments when hit.
class Asteroid {
public:
    enum Size { Large = 3, Medium = 2, Small = 1 };

    Vec2 pos;
    Vec2 vel;
    float angle = 0.0f;
    float spin = 0.0f;
    int size = Large;
    bool alive = true;

    float radius = 30.0f;
    std::vector<Vec2> verts; // normalized local vertices, approximately 1.0 long

    void Reset(float x, float y, int size, uint32_t seed);
    void Update(float dt);
    void Wrap();
    void Draw(SDL_Renderer* renderer) const;

    static float RadiusForSize(int size);
};

#endif // ASTEROID_H
