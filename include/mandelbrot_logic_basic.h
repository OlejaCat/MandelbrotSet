#ifndef MANDELBROT_LOGIC_H
#define MANDELBROT_LOGIC_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "mandelbrot_struct.h"

void calculateMandelbrotSeparated(int pitch,
                                  uint32_t* pixels,
                                  MandelbrotData* data);
void calculateIterationField(MandelbrotData* data);

#endif
