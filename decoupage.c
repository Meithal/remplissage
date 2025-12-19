//
// Created by ivo on 04/03/2025.
//

#include "decoupage.h"

typedef int OutCode;

static const int INSIDE = 0b0000;
static const int LEFT   = 0b0001;
static const int RIGHT  = 0b0010;
static const int BOTTOM = 0b0100;
static const int TOP    = 0b1000;

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)

// ASSUME THAT xmax, xmin, ymax and ymin are global constants.

static OutCode ComputeOutCode(float x, float y, float xmin, float xmax, float ymin, float ymax)
{
    OutCode code = INSIDE;  // initialised as being inside of clip window

    if (x < xmin)           // to the left of clip window
        code |= LEFT;
    else if (x > xmax)      // to the right of clip window
        code |= RIGHT;
    if (y < ymin)           // below the clip window
        code |= BOTTOM;
    else if (y > ymax)      // above the clip window
        code |= TOP;

    return code;
}

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with
// diagonal from (xmin, ymin) to (xmax, ymax).
// Todo: fenetre direction opposée
_Bool CohenSutherlandLineClip(
        float *x0, float *y0, float *x1, float *y1, float ymax, float ymin, float xmax, float xmin) {
    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    OutCode outcode0 = ComputeOutCode(*x0, *y0, xmin, xmax, ymin, ymax);
    OutCode outcode1 = ComputeOutCode(*x1, *y1, xmin, xmax, ymin, ymax);
    _Bool accept = 0;

    while (1) {
        if (!(outcode0 | outcode1)) {
            // bitwise OR is 0: both points inside window; trivially accept and exit loop
            accept = 1;
            break;
        } else if (outcode0 & outcode1) {
            // bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
            // or BOTTOM), so both must be outside window; exit loop (accept is false)
            break;
        } else {
            // failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            float x, y;

            // At least one endpoint is outside the clip rectangle; pick it.
            OutCode outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

            // Now find the intersection point;
            // use formulas:
            //   slope = (y1 - y0) / (x1 - x0)
            //   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
            //   y = y0 + slope * (xm - x0), where xm is xmin or xmax
            // No need to worry about divide-by-zero because, in each case, the
            // outcode bit being tested guarantees the denominator is non-zero
            if (outcodeOut & TOP) {           // point is above the clip window
                x = *x0 + (*x1 - *x0) * (ymax - *y0) / (*y1 - *y0);
                y = ymax;
            } else if (outcodeOut & BOTTOM) { // point is below the clip window
                x = *x0 + (*x1 - *x0) * (ymin - *y0) / (*y1 - *y0);
                y = ymin;
            } else if (outcodeOut & RIGHT) {  // point is to the right of clip window
                y = *y0 + (*y1 - *y0) * (xmax - *x0) / (*x1 - *x0);
                x = xmax;
            } else if (outcodeOut & LEFT) {   // point is to the left of clip window
                y = *y0 + (*y1 - *y0) * (xmin - *x0) / (*x1 - *x0);
                x = xmin;
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcodeOut == outcode0) {
                *x0 = x;
                *y0 = y;
                outcode0 = ComputeOutCode(*x0, *y0, xmin, xmax, ymin, ymax);
            } else {
                *x1 = x;
                *y1 = y;
                outcode1 = ComputeOutCode(*x1, *y1, xmin, xmax, ymin, ymax);
            }
        }
    }
    return accept;
}

_Bool SutherlandHogmanLineClip(
        float *x0, float *y0, float *x1, float *y1, float ymax, float ymin, float xmax, float xmin) {
    
    return 1;
}

