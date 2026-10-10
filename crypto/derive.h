#ifndef KEYGEN_DERIVE_H
#define KEYGEN_DERIVE_H

#include "curve.h"

void derive_limbs(const uint32_t gen[5], uint16_t out[15]);
void pubkey_hex(const point *base, const char *seed, char out[65]);

#endif
