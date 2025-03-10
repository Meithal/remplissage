
#define RM_MAX_SHAPES 20
#define RM_MAX_POINTS 20

extern struct shape {
    unsigned char colors[4];
    unsigned char remplissage_colors[4];
    int last_point;
    struct vec2 {
        float y;
        float x;
    } points[RM_MAX_POINTS];
} g_shapes[RM_MAX_SHAPES];
extern int g_cur_shape;

extern struct shape g_clips[RM_MAX_SHAPES];
extern int g_cur_clip;

float norm(int pixel, int width);
int tex(float coord, int width);