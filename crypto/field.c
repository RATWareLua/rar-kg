#include "field.h"
#include "gf15.h"
#include <string.h>
#include <emmintrin.h>

void fe_norm(fe *a)
{
    while (a->cnt && !a->c[a->cnt - 1])
        a->cnt--;
}

void fe_xor(fe *r, const fe *a, const fe *b)
{
    int n = a->cnt > b->cnt ? a->cnt : b->cnt;
    int shared_coefficients = a->cnt < b->cnt ? a->cnt : b->cnt;
    int i = 0;
    for (; i + 8 <= shared_coefficients; i += 8) {
        __m128i left = _mm_loadu_si128((const __m128i *)(a->c + i));
        __m128i right = _mm_loadu_si128((const __m128i *)(b->c + i));
        __m128i sum = _mm_xor_si128(left, right);
        _mm_storeu_si128((__m128i *)(r->c + i), sum);
    }
    for (; i < n; i++)
        r->c[i] = (i < a->cnt ? a->c[i] : 0) ^ (i < b->cnt ? b->c[i] : 0);
    r->cnt = n;
    fe_norm(r);
}

void fe_mul(fe *r, const fe *a, const fe *b)
{
    uint16_t t[33] = {0};
    for (int i = 0; i < a->cnt; i++) {
        for (int j = 0; j < b->cnt; j++)
            t[i + j] ^= gfmul(a->c[i], b->c[j]);
    }
    for (int i = a->cnt + b->cnt - 2; i >= 17; i--) {
        t[i-17] ^= t[i];
        t[i-14] ^= t[i];
    }
    memcpy(r->c, t, sizeof(r->c));
    r->cnt = 17;
    fe_norm(r);
}

void fe_sqr(fe *r, const fe *a)
{
    fe_mul(r, a, a);
}

void fe_div(fe *r, const fe *a, const fe *b)
{
    fe inv = *b;
    for (int i = 0; i < 253; i++) {
        fe_sqr(&inv, &inv);
        fe_mul(&inv, &inv, b);
    }
    fe_sqr(&inv, &inv); /* b^(2^255 - 2) */
    fe_mul(r, a, &inv);
}

int fe_equal(const fe *a, const fe *b)
{
    return a->cnt == b->cnt && !memcmp(a->c, b->c, a->cnt * sizeof(*a->c));
}

void fe_to_bytes(const fe *a, uint8_t out[32])
{
    memset(out, 0, 32);
    for (int i = 0; i < a->cnt; i++) {
        uint32_t v = (uint32_t)a->c[i] << ((15*i) & 7);
        int byte = 15*i / 8;
        out[byte] |= (uint8_t)v;
        out[byte+1] |= (uint8_t)(v >> 8);
        if (byte + 2 < 32)
            out[byte + 2] |= (uint8_t)(v >> 16);
    }
}
