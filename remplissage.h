//
// Created by ivo on 04/03/2025.
//

#ifndef FENETRAGE_REMPLISSAGE_REMPLISSAGE_H
#define FENETRAGE_REMPLISSAGE_REMPLISSAGE_H


void calculate_intersections(struct vec2 start, struct vec2 end, int intersections[RM_MAX_POINTS]);
_Bool is_crossing(int x, int intersections[RM_MAX_POINTS]);

#endif //FENETRAGE_REMPLISSAGE_REMPLISSAGE_H
