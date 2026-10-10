#ifndef KEYGEN_CONSTANTS_H
#define KEYGEN_CONSTANTS_H

#include "../crypto/curve.h"

typedef struct {
    point base;
    uint16_t order[16];
} curve_params;
int constants_load(const char *path, curve_params *curve);

#endif
