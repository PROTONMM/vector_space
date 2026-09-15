#ifndef VECTOR_FONT_H
#define VECTOR_FONT_H

#include <SDL2/SDL.h>

#include "vector.h"

// Single-stroke vector font. Every character is composed entirely of lines.
float VectorTextWidth(const char* text, float height);
void DrawVectorText(SDL_Renderer* renderer, const char* text, float x, float y,
                    float height, Color color);
void DrawVectorTextCentered(SDL_Renderer* renderer, const char* text, float cx,
                            float y, float height, Color color);

#endif
