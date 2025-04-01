#ifndef MANDELBROT_START_H
#define MANDELBROT_START_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

const double ZOOM_FACTOR = 1.1;
const double DEFAULT_ZOOM_FACTOR = 1.0;
const double MOVE_SPEED  = 0.1;

int startMandelbrot(SDL_Renderer* renderer, SDL_Texture* texture);

#endif // MANDELBROT_START_H
