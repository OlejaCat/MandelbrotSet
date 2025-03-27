#include "mandelbrot_logic.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <immintrin.h>

#include "constants.h"


// static ---------------------------------------------------------------------


static const int MAX_ITERATIONS = 1000;
static const double ZOOM_FACTOR = 1.1;
static const double MOVE_SPEED  = 0.1;

static const double ratio = (double)SCREEN_WIDTH / SCREEN_HEIGHT;
static const double base_width = 3.0;

static void setDefaultMandelbrot(MandelbrotData* data);
static void updateDimension(MandelbrotData* data);
static void handleInput(SDL_Event* event, MandelbrotData* data);
static int calculateColorFromPosition(int x, int y, MandelbrotData* data);
static uint32_t getColorFromIteration(int iterations, 
                               const SDL_PixelFormatDetails* format);
static void calculateMandelbrot(int pitch, 
                                uint32_t* pixels,
                                const SDL_PixelFormatDetails* format,
                                MandelbrotData* data);
static void calculateMandelbrotIntrinsics(int pitch, 
                                          uint32_t* pixels,
                                          const SDL_PixelFormatDetails* format,
                                          MandelbrotData* data);
static __m256i getColorFromIteration(__m256i iterations, 
                                     const SDL_PixelFormatDetails* format);
static __m256i calculateColorFromPositionIntrinsics(__m256d x_pixels, 
                                                    __m256d y_pixels, 
                                                    MandelbrotData* data);

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

        // calculateMandelbrot(pitch, pixels, format, &mandelbrot_data);
        calculateMandelbrotIntrinsics(pitch, pixels, format, &mandelbrot_data);

        SDL_UnlockTexture(texture);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        frame_time = SDL_GetTicks() - start_time;
        fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;

        printf("FPS: %g\n", fps);
    }

    return 0;
}


// static ---------------------------------------------------------------------


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


static void setDefaultMandelbrot(MandelbrotData* data)
{
    assert(data != NULL);

    const double zoom = 1.0;
    data->zoom = zoom;

    const double width = base_width / zoom;
    const double height = width / ratio;
    data->width = width;
    data->height = height;

    data->center_x = -0.75;
    data->center_y = 0.0;

    return;
}


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


static void calculateMandelbrotIntrinsics(int pitch, 
                                          uint32_t* pixels,
                                          const SDL_PixelFormatDetails* format,
                                          MandelbrotData* data)
{
    assert(pixels != NULL);
    assert(format != NULL);
    assert(data   != NULL);

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x+=4) 
        {
            __m256d x_pixels = _mm256_setr_pd((double)(x),
                                              (double)(x + 1),
                                              (double)(x + 2),
                                              (double)(x + 3));
            __m256d y_pixels = _mm256_set1_pd((double)y);
            __m256i iterations = calculateColorFromPositionIntrinsics(x_pixels, y_pixels, data);
            int64_t iterations_array[4] = {};
            _mm256_store_si256((__m256i*)iterations_array, iterations);
            for (int x0 = 0; x0 < 4; x0++)
            {
                uint32_t color = getColorFromIteration((int)(iterations_array[x0]), format);
                pixels[y * (pitch / 4) + x + x0] = color;
            }
        }
    }
}


//static __m256i getColorFromIteration(__m256i iterations, const SDL_PixelFormatDetails* format) 
//{
//    assert(format != NULL);
//
//    if (iterations >= MAX_ITERATIONS)
//    {
//        return SDL_MapRGBA(format, NULL, 0, 0, 0, 255);
//    }
//    
//    double t = (double)iterations / MAX_ITERATIONS;
//
//    t = pow(t, 0.5);
//
//    uint8_t r, g, b;
//    if (t < 0.25) {
//        r = 50;
//        g = 4 * t * 255;
//        b = 255;
//    } else if (t < 0.5) {
//        r = 50;
//        g = 255;
//        b = 255 - 4 * (t - 0.25) * 255;
//    } else if (t < 0.75) {
//        r = 4 * (t - 0.5) * 255;
//        g = 255;
//        b = 50;
//    } else {
//        r = 255;
//        g = 255 - 4 * (t - 0.75) * 255;
//        b = 50;
//    }
//
//    uint32_t color = SDL_MapRGBA(format, NULL, r, g, b, 255);
//    return color;
//}


static __m256i calculateColorFromPositionIntrinsics(__m256d x_pixels, 
                                                    __m256d y_pixels, 
                                                    MandelbrotData* data)
{
    assert(data != NULL);

    __m256d mandelbrot_width = _mm256_set1_pd(data->width);
    __m256d screen_width = _mm256_set1_pd((double)SCREEN_WIDTH);

    __m256d norm_x = _mm256_mul_pd(x_pixels, mandelbrot_width);
    norm_x = _mm256_div_pd(norm_x, screen_width);

    __m256d mandelbrot_height = _mm256_set1_pd(data->height);
    __m256d screen_height = _mm256_set1_pd((double)SCREEN_HEIGHT);

    __m256d norm_y = _mm256_sub_pd(screen_height, y_pixels);
    norm_y = _mm256_mul_pd(norm_y, mandelbrot_height);
    norm_y = _mm256_div_pd(norm_y, screen_height);

    __m256d twos = _mm256_set1_pd(2.0);
    mandelbrot_width  = _mm256_div_pd(mandelbrot_width, twos);
    mandelbrot_height = _mm256_div_pd(mandelbrot_height, twos);

    __m256d center_x = _mm256_set1_pd(data->center_x);
    __m256d center_y = _mm256_set1_pd(data->center_y);

    __m256d x0 = _mm256_add_pd(norm_x, center_x);
    x0 = _mm256_sub_pd(x0, mandelbrot_width);

    __m256d y0 = _mm256_add_pd(norm_y, center_y);
    y0 = _mm256_sub_pd(y0, mandelbrot_height);

    __m256d X2 = _mm256_setzero_pd();
    __m256d Y2 = _mm256_setzero_pd();

    __m256d X = _mm256_setzero_pd();
    __m256d Y = _mm256_setzero_pd();

    __m256d W = _mm256_setzero_pd();

    __m256i iteration = _mm256_setzero_si256();
    __m256d coordinates_sum = _mm256_setzero_pd();
    __m256d cmp_result = _mm256_setzero_pd();
    __m256d root_radius = _mm256_setzero_pd();
    __m256d radius_max = _mm256_set1_pd(4.0f);
    int64_t mask = 0;
    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        root_radius = _mm256_add_pd(X2, Y2); 
        cmp_result = _mm256_cmp_pd(root_radius, radius_max, _CMP_LE_OS);
        mask = _mm256_movemask_pd(cmp_result);

        if (!mask)
        {
            break;
        }
        
        X = _mm256_sub_pd(X2, Y2);
        X = _mm256_add_pd(X, x0);

        Y = _mm256_sub_pd(W, X2);
        Y = _mm256_sub_pd(Y, Y2);
        Y = _mm256_add_pd(Y, y0);

        X2 = _mm256_mul_pd(X, X);
        Y2 = _mm256_mul_pd(Y, Y);

        coordinates_sum = _mm256_add_pd(X, Y);
        W = _mm256_mul_pd(coordinates_sum, coordinates_sum);

        iteration = _mm256_sub_epi64(iteration, 
                                     _mm256_castpd_si256(cmp_result));
    }
    
    return iteration;
}         


static void calculateMandelbrot(int pitch, 
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


static void updateDimension(MandelbrotData* data)
{
    const double width = base_width / data->zoom;
    const double height = width / ratio;
    data->width = width;
    data->height = height;
}
