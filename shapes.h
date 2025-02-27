
#define RM_MAX_SHAPES 20
#define RM_MAX_POINTS 20

extern struct shape {
    float colors[4];
    int last_point;
    float points[RM_MAX_POINTS][2];
} g_shapes[RM_MAX_SHAPES];
extern int g_cur_shape;
extern int g_last_shape;