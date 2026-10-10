#ifndef KEYGEN_SHA1_H
#define KEYGEN_SHA1_H

#include <stddef.h>
#include <stdint.h>

void sha1_words(const uint8_t *p, size_t n, uint32_t w[5]);

#endif
