#ifndef KEYGEN_FIELD_H
#define KEYGEN_FIELD_H

#include <stdint.h>

/* GF((2^15)^17): t^17 + t^3 + 1 */
typedef struct {
    int cnt;
    uint16_t c[17];
} fe;

void fe_norm(fe *a);
void fe_xor(fe *r, const fe *a, const fe *b);
void fe_mul(fe *r, const fe *a, const fe *b);
void fe_sqr(fe *r, const fe *a);
void fe_div(fe *r, const fe *a, const fe *b);
int fe_equal(const fe *a, const fe *b);
void fe_to_bytes(const fe *a, uint8_t out[32]);

#endif
