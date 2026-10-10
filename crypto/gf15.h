#ifndef KEYGEN_GF15_H
#define KEYGEN_GF15_H

#include <stdint.h>

/* GF(2^15): x^15 + x + 1 */
void gf_init(void);
uint16_t gfmul(uint16_t a, uint16_t b);

#endif
