#ifndef KEYGEN_RNG_H
#define KEYGEN_RNG_H

#include "../crypto/chacha20.h"

int rng_init(chacha20_rng *rng);

#endif
