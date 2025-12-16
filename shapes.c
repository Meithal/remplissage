#include "shapes.h"

struct shape g_shapes[RM_MAX_SHAPES] = {
        {
            .colors = {128, 8 , 108, 255},
            .remplissage_colors = {255, 255, 255, 255},
            .last_point = 0,
            .points = {0}
        }
};

int g_active_shape = 0;
int g_last_shape = 1; // borne superieure exclusive des shapes qu'on doit dessiner

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

float convert_to_ratio(float value, float low_bound, float high_bound, float target_low, float target_high)
{
    float fenetre = high_bound - low_bound;
    float fenetre_cible = target_high - target_low;
    
    float ratio = (value - low_bound) / fenetre; // 0-1
    float ratio_cible = ratio * fenetre_cible;
    float final = ratio_cible + target_low;
    
    return final;
}
