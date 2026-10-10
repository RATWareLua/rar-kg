#ifndef KEYGEN_CRC32_H
#define KEYGEN_CRC32_H

#include <stdint.h>

void crc32_init(void);
uint32_t crc32_string(uint32_t crc, const char *s);

#endif
