#ifndef KEYGEN_BIGINT_H
#define KEYGEN_BIGINT_H

#include <stdint.h>

/* LE16 limbs */
typedef struct {
    int n;
    uint16_t w[40];
} bn;

void bn_norm(bn *a);
bn bn_from_u16limbs(const uint16_t *w, int n);
void bn_from_bytes(bn *r, const uint8_t *p, int n);
int bn_cmp(const bn *a, const bn *b);
bn bn_add(const bn *a, const bn *b);
bn bn_sub(const bn *a, const bn *b);
bn bn_mod(const bn *x, const bn *n);
bn bn_mulmod(const bn *a, const bn *b, const bn *n);
void bn_to_hex(const bn *a, int digits, char *out);

#endif
