#include "chacha20.h"
#include <string.h>

static uint32_t rotate_left(uint32_t value, int bits)
{
    return (value << bits) | (value >> (32 - bits));
}

static void quarter_round(uint32_t state[16], int a, int b, int c, int d)
{
    state[a] += state[b];
    state[d] = rotate_left(state[d] ^ state[a], 16);
    state[c] += state[d];
    state[b] = rotate_left(state[b] ^ state[c], 12);
    state[a] += state[b];
    state[d] = rotate_left(state[d] ^ state[a], 8);
    state[c] += state[d];
    state[b] = rotate_left(state[b] ^ state[c], 7);
}

void chacha20_init(chacha20_rng *rng, const uint32_t key[8], const uint32_t nonce[3])
{
    *rng = (chacha20_rng){
        .state = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574},
        .used = 64
    };
    memcpy(rng->state + 4, key, 8 * sizeof(*key));
    memcpy(rng->state + 13, nonce, 3 * sizeof(*nonce));
}

static void generate_block(chacha20_rng *rng)
{
    uint32_t *state = rng->block;
    memcpy(state, rng->state, sizeof(rng->state));
    for (int round = 0; round < 10; round++) {
        quarter_round(state, 0, 4, 8, 12);
        quarter_round(state, 1, 5, 9, 13);
        quarter_round(state, 2, 6, 10, 14);
        quarter_round(state, 3, 7, 11, 15);

        quarter_round(state, 0, 5, 10, 15);
        quarter_round(state, 1, 6, 11, 12);
        quarter_round(state, 2, 7, 8, 13);
        quarter_round(state, 3, 4, 9, 14);
    }
    for (int i = 0; i < 16; i++)
        state[i] += rng->state[i];

    rng->state[12]++;
    rng->exhausted = rng->state[12] == 0;
    rng->used = 0;
}

int chacha20_read(chacha20_rng *rng, void *out, size_t size)
{
    uint8_t *output = out;
    while (size) {
        if (rng->used == 64) {
            if (rng->exhausted)
                return 0;
            generate_block(rng);
        }
        size_t take = 64 - rng->used;
        if (take > size)
            take = size;
        memcpy(output, (const uint8_t *)rng->block + rng->used, take);
        rng->used += (unsigned)take;
        output += take;
        size -= take;
    }
    return 1;
}
