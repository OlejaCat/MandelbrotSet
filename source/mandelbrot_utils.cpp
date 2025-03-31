#include "mandelbrot_utils.h"

#include <stdio.h>
#include <assert.h>


void setDefaultMandelbrot(MandelbrotData* data)
{
    assert(data != NULL);

    data->zoom = DEFAULT_ZOOM;

    const double width = DEFAULT_WIDTH / DEFAULT_ZOOM;
    const double height = width / SCREEN_RATIO;
    data->width = width;
    data->height = height;

    data->center_x = DEFAULT_CENTER_X;
    data->center_y = DEFAULT_CENTER_Y;
}


void updateDimension(MandelbrotData* data)
{
    const double width = DEFAULT_WIDTH / data->zoom;
    const double height = width / SCREEN_RATIO;
    data->width = width;
    data->height = height;
}

