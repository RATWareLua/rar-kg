#ifndef KEYGEN_KEY_H
#define KEYGEN_KEY_H

#include "../pe/constants.h"
#include "../crypto/chacha20.h"

typedef struct {
    char uid[21];
    char data[379];
} key_data;
int key_generate(const curve_params *curve, const char *username, const char *license,
                 chacha20_rng *rng, key_data *key);

#endif
