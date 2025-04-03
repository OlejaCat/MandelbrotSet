#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <math.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "screen_constants.h"
#include "mandelbrot_start.h"


int main(int argc, char* argv[])
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                     "Couldn't initialize SDL: %s\n", 
                     SDL_GetError());
        return 1; 
    }
    
    SDL_Window* window = SDL_CreateWindow(
            "Mandelbrot Set", 
            SCREEN_WIDTH, 
            SCREEN_HEIGHT, 
            SDL_WINDOW_OPENGL
    );

    if (!window) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                     "Could not create window: %s\n", 
                     SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                     "Could not create renderer: %s\n", 
                     SDL_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT
    );
    if (!texture)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                     "Could not create texture: %s\n", 
                     SDL_GetError());
        return 1;
    }

    int return_code = startMandelbrot(argc, argv, renderer, texture);
    if (return_code)
    {
        fprintf(stderr, "Error while calculating mandelbrot set\n");
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}

