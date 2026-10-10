#ifndef KEYGEN_CHACHA20_H
#define KEYGEN_CHACHA20_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t state[16];
    uint32_t block[16];
    unsigned used;
    int exhausted;
} chacha20_rng;

void chacha20_init(chacha20_rng *rng, const uint32_t key[8], const uint32_t nonce[3]);
int chacha20_read(chacha20_rng *rng, void *out, size_t size);

#endif
