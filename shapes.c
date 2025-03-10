#include "shapes.h"

struct shape g_shapes[RM_MAX_SHAPES] = {
        {
            .colors = {128, 8 , 108, 255},
            .remplissage_colors = {255, 255, 255, 255},
            .last_point = 0,
            .points = {0}
        }
};

int g_cur_shape = 0;

struct shape g_clips[RM_MAX_SHAPES] = {
        {
                .colors = {8, 128 , 108, 255},
                .remplissage_colors = {255, 255, 255, 255},
                .last_point = 0,
                .points = {}
        }
};

int g_cur_clip = 0;

float norm(int pixel, int width) {
    return (float)pixel / width * 2 - 1;
}

int tex(float coord, int width)
{
    return (coord * 0.5f + 1) * width;
}