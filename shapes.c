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
