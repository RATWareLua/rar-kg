#include "pe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t pe_read32(const pe_image *image, long offset)
{
    uint32_t value;
    memcpy(&value, image->data + offset, sizeof(value));
    return value;
}

void pe_close(pe_image *image)
{
    free(image->data);
    *image = (pe_image){0};
}

static int parse_header(pe_image *image)
{
    if ((pe_read32(image, 0) & 0xffff) != 0x5a4d)
        return 0;
    uint32_t pe_offset = pe_read32(image, 0x3c);
    if (pe_offset > (uint32_t)image->size - 64 || pe_read32(image, pe_offset) != 0x4550 ||
        (pe_read32(image, pe_offset + 4) & 0xffff) != 0x8664)
        return 0;

    uint32_t optional_size = pe_read32(image, pe_offset + 0x14) & 0xffff;
    image->section_count = pe_read32(image, pe_offset + 6) & 0xffff;
    if (image->section_count > 16)
        image->section_count = 16;
    uint32_t section_table = pe_offset + 0x18 + optional_size;
    if (optional_size < 32 || (pe_read32(image, pe_offset + 0x18) & 0xffff) != 0x20b ||
        section_table + 40 * image->section_count > (uint32_t)image->size)
        return 0;

    memcpy(&image->image_base, image->data + pe_offset + 0x30, sizeof(image->image_base));
    for (int i = 0; i < image->section_count; i++) {
        long offset = section_table + 40 * i;
        pe_section *section = &image->sections[i];
        section->rva = pe_read32(image, offset + 12);
        section->size = pe_read32(image, offset + 16);
        section->offset = pe_read32(image, offset + 20);
        if (section->offset > (uint32_t)image->size ||
            section->size > (uint32_t)image->size - section->offset)
            return 0;
    }
    return 1;
}

int pe_open(pe_image *image, const char *path)
{
    *image = (pe_image){0};
    FILE *file = fopen(path, "rb");
    if (!file)
        return 0;

    int ok = 0;
    if (fseek(file, 0, SEEK_END))
        goto cleanup;
    image->size = ftell(file);
    if (image->size < 64 || fseek(file, 0, SEEK_SET))
        goto cleanup;
    image->data = malloc((size_t)image->size);
    if (!image->data || fread(image->data, 1, (size_t)image->size, file) != (size_t)image->size)
        goto cleanup;
    ok = parse_header(image);
cleanup:
    fclose(file);
    if (!ok)
        pe_close(image);
    return ok;
}

long pe_target(const pe_image *image, long end, int32_t displacement)
{
    for (int i = 0; i < image->section_count; i++) {
        const pe_section *section = &image->sections[i];
        if (end < (long)section->offset || end >= (long)section->offset + (long)section->size)
            continue;
        uint32_t rva = section->rva + (uint32_t)(end - section->offset) + (uint32_t)displacement;
        for (int j = 0; j < image->section_count; j++) {
            section = &image->sections[j];
            if (rva >= section->rva && rva - section->rva < section->size)
                return (long)(rva - section->rva + section->offset);
        }
        return -1;
    }
    return -1;
}
