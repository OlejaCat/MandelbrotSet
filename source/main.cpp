#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <math.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 1000
#define MAX_ITERATIONS 250
#define ZOOM_FACTOR 1.1
#define MOVE_SPEED 0.1

const double ratio = (double)SCREEN_WIDTH / SCREEN_HEIGHT;
const double base_width = 3.0;

typedef struct MandelbrotData
{
    double zoom;
    double center_x;
    double center_y;
    double width;
    double height;    
} MandelbrotData;

void setDefaultMandelbrot(MandelbrotData* data);
void updateDimension(MandelbrotData* data);
void handleInput(SDL_Event* event, MandelbrotData* data);
int calculateColorFromPosition(int x, int y, MandelbrotData* data);
uint32_t getColorFromIteration(int iterations, const SDL_PixelFormatDetails* format);
SDL_Palette* create_sdl3_palette(int max_iterations);

int main()
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

    uint32_t* pixels;
    int pitch;

    const SDL_PixelFormatDetails* format = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);

    MandelbrotData mandelbrot_data = {};
    setDefaultMandelbrot(&mandelbrot_data);

    bool done = false;
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

        SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++) 
            {
                int round = calculateColorFromPosition(x, y, &mandelbrot_data);
                uint32_t color = getColorFromIteration(round, format);
                pixels[y * (pitch / 4) + x] = color;
            }
        }

        SDL_UnlockTexture(texture);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}

void setDefaultMandelbrot(MandelbrotData* data)
{
    const double zoom = 3.0;
    data->zoom = zoom;

    const double width = base_width / zoom;
    const double height = width / ratio;
    data->width = width;
    data->height = height;

    data->center_x = -0.75;
    data->center_y = 0.0;
}


int calculateColorFromPosition(int             x_pixel, 
                               int             y_pixel, 
                               MandelbrotData* data)
{
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


uint32_t getColorFromIteration(int iterations, const SDL_PixelFormatDetails* format) 
{
    if (iterations >= MAX_ITERATIONS)
    {
        return SDL_MapRGBA(format, NULL, 0, 0, 0, 255);
    }
    
    double t = (double)iterations / MAX_ITERATIONS;

    t = pow(t, 0.5);

    Uint8 r, g, b;
    if (t < 0.25) {
        r = 0;
        g = 4 * t * 255;
        b = 255;
    } else if (t < 0.5) {
        r = 0;
        g = 255;
        b = 255 - 4 * (t - 0.25) * 255;
    } else if (t < 0.75) {
        r = 4 * (t - 0.5) * 255;
        g = 255;
        b = 0;
    } else {
        r = 255;
        g = 255 - 4 * (t - 0.75) * 255;
        b = 0;
    }

    uint32_t color = SDL_MapRGBA(format, NULL, r, g, b, 255);
    return color;
}



void handleInput(SDL_Event* event, MandelbrotData* data)
{
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

            case SDLK_UP:
                data->center_y -= data->height * MOVE_SPEED;
                break;

            case SDLK_DOWN:
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
            float mouse_x, mouse_y; 
            SDL_GetMouseState(&mouse_x, &mouse_y);

            double norm_x = (mouse_x / (double)SCREEN_WIDTH) * data->width;
            double norm_y = ((SCREEN_HEIGHT - mouse_y) / (double)SCREEN_HEIGHT) * data->height;

            data->center_x = data->center_x + (norm_x - data->width / 2);
            data->center_y = data->center_y + (norm_y - data->height / 2);                
        }
    }
}


void updateDimension(MandelbrotData* data)
{
    const double width = base_width / data->zoom;
    const double height = width / ratio;
    data->width = width;
    data->height = height;
}
