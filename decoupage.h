//
// Created by ivo on 04/03/2025.
//

#ifndef FENETRAGE_REMPLISSAGE_DECOUPAGE_H
#define FENETRAGE_REMPLISSAGE_DECOUPAGE_H

_Bool CohenSutherlandLineClip(float *x0, float *y0, float *x1, float *y1, float ymax, float ymin, float xmax,
                              float xmin);
_Bool SutherlandHogmanLineClip(float *x0, float *y0, float *x1, float *y1, float ymax, float ymin, float xmax,
                              float xmin);

#endif //FENETRAGE_REMPLISSAGE_DECOUPAGE_H
