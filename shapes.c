#include "shapes.h"

struct shape g_shapes[RM_MAX_SHAPES] = {
        {
            {128, 8 , 108, 255},
            0,
            {}
        }
};

int g_cur_shape = 0;

struct shape g_clips[RM_MAX_SHAPES] = {
        {
                {8, 1288 , 108, 255},
                0,
                {}
        }
};
int g_cur_clip = 0;
