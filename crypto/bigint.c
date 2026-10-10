#include "bigint.h"
#include <string.h>

void bn_norm(bn *a)
{
    while (a->n && !a->w[a->n - 1])
        a->n--;
}

bn bn_from_u16limbs(const uint16_t *w, int n)
{
    bn r = {0};
    memcpy(r.w, w, n * sizeof(*w));
    r.n = n;
    bn_norm(&r);
    return r;
}

void bn_from_bytes(bn *r, const uint8_t *p, int n)
{
    *r = (bn){0};
    for (int i = 0; i < n; i++)
        r->w[i / 2] |= (uint16_t)p[i] << (8 * (i % 2));
    r->n = (n + 1) / 2;
    bn_norm(r);
}

int bn_cmp(const bn *a, const bn *b)
{
    if (a->n != b->n)
        return a->n < b->n ? -1 : 1;
    for (int i = a->n - 1; i >= 0; i--) {
        if (a->w[i] != b->w[i])
            return a->w[i] < b->w[i] ? -1 : 1;
    }
    return 0;
}

bn bn_add(const bn *a, const bn *b)
{
    bn r = {0};
    uint32_t carry = 0;
    int n = a->n > b->n ? a->n : b->n;
    for (int i = 0; i < n || carry; i++) {
        uint32_t left = i < a->n ? a->w[i] : 0;
        uint32_t right = i < b->n ? b->w[i] : 0;
        uint32_t sum = left + right + carry;
        r.w[i] = (uint16_t)sum;
        carry = sum >> 16;
        r.n++;
    }
    return r;
}

bn bn_sub(const bn *a, const bn *b) /* a >= b */
{
    bn r = {0};
    int borrow = 0;
    for (int i = 0; i < a->n; i++) {
        int right = i < b->n ? b->w[i] : 0;
        int difference = a->w[i] - right - borrow;
        r.w[i] = (uint16_t)difference;
        borrow = difference < 0;
    }
    r.n = a->n;
    bn_norm(&r);
    return r;
}

bn bn_mod(const bn *x, const bn *n)
{
    bn r = {0};
    for (int i = x->n * 16 - 1; i >= 0; i--) {
        uint32_t carry = (x->w[i / 16] >> (i % 16)) & 1;
        int j;
        for (j = 0; j < r.n || carry; j++) {
            uint32_t shifted = ((uint32_t)r.w[j] << 1) | carry;
            r.w[j] = (uint16_t)shifted;
            carry = shifted >> 16;
        }
        if (j > r.n)
            r.n = j;
        bn_norm(&r);
        if (bn_cmp(&r, n) >= 0)
            r = bn_sub(&r, n);
    }
    return r;
}

bn bn_mulmod(const bn *a, const bn *b, const bn *n)
{
    bn p = {0};
    for (int i = 0; i < a->n; i++) {
        uint32_t carry = 0;
        for (int j = 0; j < b->n || carry; j++) {
            uint32_t right = j < b->n ? b->w[j] : 0;
            uint32_t product = p.w[i + j] + (uint32_t)a->w[i] * right + carry;
            p.w[i + j] = (uint16_t)product;
            carry = product >> 16;
        }
    }
    p.n = 40;
    bn_norm(&p);
    return bn_mod(&p, n);
}

void bn_to_hex(const bn *a, int digits, char *out)
{
    for (int i = 0; i < digits; i++) {
        int nibble = digits - 1 - i;
        int limb = nibble / 4;
        unsigned value = 0;
        if (limb < a->n) {
            int shift = 4 * (nibble % 4);
            value = (a->w[limb] >> shift) & 15;
        }
        out[i] = "0123456789abcdef"[value];
    }
    out[digits] = 0;
}
