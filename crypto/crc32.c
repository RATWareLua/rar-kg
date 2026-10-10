#include "crc32.h"

static uint32_t crc_table[256];

void crc32_init(void)
{
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c >> 1) ^ (c & 1 ? 0xedb88320u : 0);
        crc_table[i] = c;
    }
}

uint32_t crc32_string(uint32_t crc, const char *s)
{
    for (; *s; s++)
        crc = crc_table[(crc ^ (uint8_t)*s) & 255] ^ (crc >> 8);
    return crc;
}
