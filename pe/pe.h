#ifndef KEYGEN_PE_H
#define KEYGEN_PE_H

#include <stdint.h>

typedef struct {
    uint32_t rva;
    uint32_t size;
    uint32_t offset;
} pe_section;
typedef struct {
    uint8_t *data;
    long size;
    uint64_t image_base;
    int section_count;
    pe_section sections[16];
} pe_image;

int pe_open(pe_image *image, const char *path);
void pe_close(pe_image *image);
uint32_t pe_read32(const pe_image *image, long offset);
long pe_target(const pe_image *image, long end, int32_t displacement);

#endif
