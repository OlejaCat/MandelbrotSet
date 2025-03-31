#include "mandelbrot_logic_basic.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <assert.h>
#include <math.h>

#include "screen_constants.h"
#include "mandelbrot_utils.h"


// static ---------------------------------------------------------------------


static int calculateColorFromPosition(int x, int y, MandelbrotData* data);
static uint32_t getColorFromIteration(int iterations, 
                               const SDL_PixelFormatDetails* format);


// public ---------------------------------------------------------------------


void calculateMandelbrot(int pitch, 
                         uint32_t* pixels,
                         const SDL_PixelFormatDetails* format,
                         MandelbrotData* data)
{
    assert(pixels != NULL);
    assert(format != NULL);
    assert(data   != NULL);

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++) 
        {
            int round = calculateColorFromPosition(x, y, data);
            uint32_t color = getColorFromIteration(round, format);
            pixels[y * (pitch / 4) + x] = color;
        }
    }
}


// static ---------------------------------------------------------------------


static int calculateColorFromPosition(int x_pixel, 
                                      int y_pixel, 
                                      MandelbrotData* data)
{
    assert(data != NULL);

    double norm_x = (x_pixel / (double)SCREEN_WIDTH) * data->width;
    double norm_y = ((SCREEN_HEIGHT - y_pixel) / (double)SCREEN_HEIGHT) * data->height;

    const double x0 = norm_x - (data->width / 2) + data->center_x;
    const double y0 = norm_y - (data->height / 2) + data->center_y;
    double X2 = 0.0;
    double Y2 = 0.0;

    double X = 0.0;
    double Y = 0.0;

    double W = 0.0;

    int iteration = 0;
    while (X2 + Y2 <= 4 && iteration < MAX_ITERATIONS)
    {
        X = X2 - Y2 + x0;
        Y = W - X2 - Y2 + y0;
        X2 = X * X;
        Y2 = Y * Y;
        W = (X + Y) * (X + Y);
        iteration++;
    }
    
    return iteration;
}         


static uint32_t getColorFromIteration(int iterations, const SDL_PixelFormatDetails* format) 
{
    assert(format != NULL);

    if (iterations >= MAX_ITERATIONS)
    {
        return SDL_MapRGBA(format, NULL, 0, 0, 0, 255);
    }
    
    double t = (double)iterations / MAX_ITERATIONS;

    t = pow(t, 0.5);

    uint8_t r, g, b;
    if (t < 0.25) {
        r = 50;
        g = 4 * t * 255;
        b = 255;
    } else if (t < 0.5) {
        r = 50;
        g = 255;
        b = 255 - 4 * (t - 0.25) * 255;
    } else if (t < 0.75) {
        r = 4 * (t - 0.5) * 255;
        g = 255;
        b = 50;
    } else {
        r = 255;
        g = 255 - 4 * (t - 0.75) * 255;
        b = 50;
    }

    uint32_t color = SDL_MapRGBA(format, NULL, r, g, b, 255);
    return color;
}

