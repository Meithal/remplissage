#include "shapes.h"

struct shape g_shapes[RM_MAX_SHAPES] = {
        {
            {128, 8 , 108, 255},
            3,
            {}
        }
};

int g_cur_shape = 0;
int g_last_shape = 0;
