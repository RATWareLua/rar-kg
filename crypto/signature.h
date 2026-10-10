#ifndef KEYGEN_SIGNATURE_H
#define KEYGEN_SIGNATURE_H

#include "curve.h"
#include "chacha20.h"

int sign_msg(const point *base, const uint16_t order[16], const uint16_t d_limbs[15],
             const char *msg, chacha20_rng *rng, char s_hex[61], char r_hex[61]);

#endif
