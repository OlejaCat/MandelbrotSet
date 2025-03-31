#include "mandelbrot_start.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "mandelbrot_utils.h"
#include "screen_constants.h"
#include "mandelbrot_logic_basic.h"
#include "mandelbrot_logic_intrinsics.h"


// static ----------------------------------------------------------------------


static void handleInput(SDL_Event* event, MandelbrotData* data);


// public ---------------------------------------------------------------------


int startMandelbrot(SDL_Renderer* renderer, SDL_Texture* texture)
{
    assert(renderer != NULL);
    assert(texture  != NULL);

    uint32_t* pixels = NULL;
    int pitch = 0;

    const SDL_PixelFormatDetails* format = NULL;
    format = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);

    MandelbrotData mandelbrot_data = {};
    setDefaultMandelbrot(&mandelbrot_data);

    bool done = false;
    uint64_t start_time = 0;
    double fps = 0;
    uint64_t frame_time = 0;
    while (!done)
    {
        SDL_Event event; 

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT) 
            {
                done = true; 
            }
            handleInput(&event, &mandelbrot_data);
        }

        start_time = SDL_GetTicks();

        SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

        calculateMandelbrot(pitch, pixels, format, &mandelbrot_data);
        // calculateMandelbrotIntrinsics(pitch, pixels, format, &mandelbrot_data);

        SDL_UnlockTexture(texture);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        frame_time = SDL_GetTicks() - start_time;
        fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;
    }

    return 0;
}


// static ----------------------------------------------------------------------


static void handleInput(SDL_Event* event, MandelbrotData* data)
{
    assert(event != NULL);
    assert(data  != NULL);

    if (event->type == SDL_EVENT_KEY_DOWN)
    {
        switch (event->key.key) 
        {
            case SDLK_EQUALS:
                data->zoom *= ZOOM_FACTOR;
                updateDimension(data);
                break;

            case SDLK_MINUS:
                data->zoom /= ZOOM_FACTOR;
                updateDimension(data);
                break;

            case SDLK_RIGHT:
                data->center_x += data->width * MOVE_SPEED;
                break;

            case SDLK_LEFT:
                data->center_x -= data->width * MOVE_SPEED;
                break;

            case SDLK_DOWN:
                data->center_y -= data->height * MOVE_SPEED;
                break;

            case SDLK_UP:
                data->center_y += data->height * MOVE_SPEED;
                break;

            default:
                break;
        }
    } 
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        if (event->button.button == SDL_BUTTON_LEFT) 
        {
            float mouse_x, 
                  mouse_y; 
            SDL_GetMouseState(&mouse_x, &mouse_y);

            double norm_x = (mouse_x / (double)SCREEN_WIDTH) * data->width;
            double norm_y = ((SCREEN_HEIGHT - mouse_y) / (double)SCREEN_HEIGHT) * data->height;

            data->center_x = data->center_x + (norm_x - data->width / 2);
            data->center_y = data->center_y + (norm_y - data->height / 2);                
        }
    }
}

