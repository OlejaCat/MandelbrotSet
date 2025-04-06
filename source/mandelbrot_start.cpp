#include "mandelbrot_start.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mandelbrot_utils.h"
#include "screen_constants.h"
#include "mandelbrot_logic_basic.h"
#include "mandelbrot_logic_intrinsics.h"
#include "mandelbrot_logic_array.h"


// static ----------------------------------------------------------------------


typedef void (*MandelbrotFunction)(int pitch, uint32_t* pixels, MandelbrotData* data);

static void handleInput(SDL_Event* event, MandelbrotData* data);


// public ---------------------------------------------------------------------


int startMandelbrot(int argc, char* argv[],
                    SDL_Renderer* renderer, 
                    SDL_Texture*  texture)
{
    assert(renderer != NULL);
    assert(texture  != NULL);

    MandelbrotFunction mandelbrot_func = calculateMandelbrotIntrinsicsSeparated;
    if (argc == 2)
    {
        if (!strcmp(argv[1], "--basic"))
        {
            mandelbrot_func = calculateMandelbrotSeparated; 
        }
        else if (!strcmp(argv[1], "--array"))
        {
            mandelbrot_func = calculateMandelbrotArraySeparated;
        }
        else if (!strcmp(argv[1], "--simd"))
        {
            mandelbrot_func = calculateMandelbrotIntrinsicsSeparated; 
        }
        else
        {
            printf("Вы ничего не выбрали... значит будет самая быстрая версия\n");
        }
    }

    uint32_t* pixels = (uint32_t*)SDL_aligned_alloc(32, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    int pitch = SCREEN_WIDTH * sizeof(uint32_t);

    MandelbrotData mandelbrot_data = {};
    setDefaultMandelbrot(&mandelbrot_data);

    bool done = false;
    //uint64_t start_time = 0;
    //double fps = 0;
    //uint64_t frame_time = 0;
    while (!done)
    {
        SDL_Event event; 

        while (SDL_PollEvent(&event))
        {
            if (event.type    == SDL_EVENT_QUIT
             || event.key.key == SDLK_Q) 
            {
                done = true; 
            }
            handleInput(&event, &mandelbrot_data);
        }

        //start_time = SDL_GetTicks();

        mandelbrot_func(pitch, pixels, &mandelbrot_data);
        if (!SDL_UpdateTexture(texture, NULL, pixels, pitch)) 
        {
            printf("Texture update failed: %s\n", SDL_GetError());
        }

        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        //frame_time = SDL_GetTicks() - start_time;
        //fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;
        //printf("%.1f\n", fps);
    }

    free(mandelbrot_data.iterations_per_pixel);
    SDL_aligned_free(pixels);

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
            float mouse_x;
            float mouse_y; 
            SDL_GetMouseState(&mouse_x, &mouse_y);

            double norm_x = (mouse_x / (double)SCREEN_WIDTH) * data->width;
            double norm_y = ((SCREEN_HEIGHT - mouse_y) / (double)SCREEN_HEIGHT) * data->height;

            data->center_x = data->center_x + (norm_x - data->width / 2);
            data->center_y = data->center_y + (norm_y - data->height / 2);                
        }
    }
}

