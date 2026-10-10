#include "curve.h"

int point_is_inf(const point *p)
{
    return !p->x.cnt && !p->y.cnt;
}

static point point_double(const point *p)
{
    point result = {0};
    fe slope, x_squared;

    fe_div(&slope, &p->y, &p->x);
    fe_xor(&slope, &slope, &p->x);
    fe_sqr(&result.x, &slope);
    fe_xor(&result.x, &result.x, &slope);

    fe_sqr(&x_squared, &p->x);
    fe_mul(&result.y, &slope, &result.x);
    fe_xor(&result.y, &result.y, &x_squared);
    fe_xor(&result.y, &result.y, &result.x);
    return result;
}

point point_add(const point *p, const point *q)
{
    if (point_is_inf(p))
        return *q;
    if (point_is_inf(q))
        return *p;
    if (fe_equal(&p->x, &q->x)) {
        if (fe_equal(&p->y, &q->y))
            return point_double(p);
        return (point){0};
    }

    point result = {0};
    fe x_difference, y_difference, slope, x_sum;
    fe_xor(&x_difference, &p->x, &q->x);
    fe_xor(&y_difference, &p->y, &q->y);
    fe_div(&slope, &y_difference, &x_difference);

    fe_sqr(&result.x, &slope);
    fe_xor(&result.x, &result.x, &slope);
    fe_xor(&result.x, &result.x, &x_difference);

    fe_xor(&x_sum, &p->x, &result.x);
    fe_mul(&result.y, &slope, &x_sum);
    fe_xor(&result.y, &result.y, &p->y);
    fe_xor(&result.y, &result.y, &result.x);
    return result;
}

point scalar_mul(const point *p, const uint16_t *w, int n)
{
    point r = {0};
    for (int i = n*16 - 1; i >= 0; i--) {
        r = point_add(&r, &r);
        if ((w[i / 16] >> (i % 16)) & 1)
            r = point_add(&r, p);
    }
    return r;
}

void point_compress(const point *p, uint8_t out[32])
{
    fe z;
    uint8_t x[32];
    fe_div(&z, &p->y, &p->x);
    fe_to_bytes(&p->x, x);
    uint32_t carry = z.c[0] & 1;
    for (int i = 0; i < 32; i++) {
        uint32_t v = (uint32_t)x[i] << 1 | carry;
        out[i] = (uint8_t)v;
        carry = v >> 8;
    }
}
