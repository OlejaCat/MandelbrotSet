#include "mandelbrot_utils.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>


// static ----------------------------------------------------------------------


void setMandelbrotPalette(MandelbrotData* data);


// public ----------------------------------------------------------------------


int setDefaultMandelbrot(MandelbrotData* data)
{
    assert(data != NULL);

    data->zoom = DEFAULT_ZOOM;

    const double width = DEFAULT_WIDTH / DEFAULT_ZOOM;
    const double height = width / SCREEN_RATIO;
    data->width = width;
    data->height = height;

    data->center_x = DEFAULT_CENTER_X;
    data->center_y = DEFAULT_CENTER_Y;

    data->iterations_per_pixel = (int*)aligned_alloc(32, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    
    if (!data->iterations_per_pixel)
    {
        fprintf(stderr, "Error while allocating memory for iterations field");
        return 1;
    }

    setMandelbrotPalette(data);

    return 0;
}


void updateDimension(MandelbrotData* data)
{
    const double width = DEFAULT_WIDTH / data->zoom;
    const double height = width / SCREEN_RATIO;
    data->width = width;
    data->height = height;
}


// public ----------------------------------------------------------------------


void setMandelbrotPalette(MandelbrotData* data)
{
    // задаем формат в котором будет храниться палитра
    const SDL_PixelFormatDetails* format = NULL;
    format = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);

    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        float t = i / (float)(MAX_ITERATIONS - 1);
        uint8_t r = 255 * sin(5 * (1 - t) * M_PI);
        uint8_t g = 255 * cos(3 * (1 - t) * M_PI);
        uint8_t b = 255 * sin(7 * (1 - t) * M_PI);
        // функция SDL_MapRGBA переводит переменные r, g, b в правильный формат
        data->colors[i] = SDL_MapRGBA(format, NULL, r, g, b, 255);
    }
}


