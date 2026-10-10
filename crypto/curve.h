#ifndef KEYGEN_CURVE_H
#define KEYGEN_CURVE_H

#include "field.h"

/* y^2 + xy = x^3 + b; (0, 0) = infinity */
typedef struct {
    fe x;
    fe y;
} point;

int point_is_inf(const point *p);
point point_add(const point *p, const point *q);
point scalar_mul(const point *p, const uint16_t *w, int n);
void point_compress(const point *p, uint8_t out[32]);

#endif
