#include "scan.h"
#include <string.h>
#include <emmintrin.h>

static void add_ref(const pe_image *image, long end, long *refs, int *count)
{
    long target = pe_target(image, end, (int32_t)pe_read32(image, end - 4));   /* disp32 = последние 4 байта */
    if (target >= 0 && *count < SCAN_REF_LIMIT)
        refs[(*count)++] = target;
}

static void scan_ref(const pe_image *image, long offset, long *refs, int *count)
{
    /* опкоды с ModRM, допускающие RIP-relative операнд (lea/mov/add/sub/xor/cmp) */
    static const uint8_t opcodes[] = {
        0x8d, 0x8b, 0x89, 0x8a, 0x88, 0x3b, 0x39,
        0x85, 0x03, 0x01, 0x2b, 0x29, 0x33, 0x31
    };
    /* SSE-загрузки/сохранения (префикс 0F xx): movups/movsd/movaps/movdqu */
    static const uint8_t sse_opcodes[] = {0x10, 0x11, 0x28, 0x29, 0x6f, 0x7f};
    const uint8_t *data = image->data;
    uint8_t opcode = data[offset];
    int has_rex = opcode >= 0x40 && opcode <= 0x4f;   /* REX-префикс */
    long modrm_offset = offset + 1 + has_rex;
    if (memchr(opcodes, data[offset + has_rex], sizeof(opcodes)) &&
        (data[modrm_offset] & 0xc7) == 5) {           /* ModRM: mod=00, rm=101 -> RIP-relative */
        add_ref(image, modrm_offset + 5, refs, count);
    }

    int prefix_size = 0;   /* длина префикса до ModRM: 0F=2, VEX2(C5)=3, VEX3(C4)=4, EVEX(62)=5 */
    if (opcode == 0x0f && memchr(sse_opcodes, data[offset + 1], sizeof(sse_opcodes)))
        prefix_size = 2;
    else if (opcode == 0xc5)
        prefix_size = 3;
    else if (opcode == 0xc4)
        prefix_size = 4;
    else if (opcode == 0x62)
        prefix_size = 5;

    if (prefix_size && (data[offset + prefix_size] & 0xc7) == 5)   /* ModRM: mod=00, rm=101 */
        add_ref(image, offset + prefix_size + 5, refs, count);
}

static uint32_t candidate_mask(const uint8_t *data)
{
    /* векторный предфильтр: байты ModRM с mod=00, rm=101 в блоке 16 байт */
    __m128i modrm_bits = _mm_set1_epi8((char)0xc7);
    __m128i rip_pattern = _mm_set1_epi8(5);
    __m128i first = _mm_loadu_si128((const __m128i *)(data + 1));
    __m128i second = _mm_loadu_si128((const __m128i *)(data + 5));
    __m128i first_matches = _mm_cmpeq_epi8(_mm_and_si128(first, modrm_bits), rip_pattern);
    __m128i second_matches = _mm_cmpeq_epi8(_mm_and_si128(second, modrm_bits), rip_pattern);
    uint32_t first_mask = (uint32_t)_mm_movemask_epi8(first_matches);
    uint32_t second_mask = (uint32_t)_mm_movemask_epi8(second_matches);
    uint32_t modrm_mask = first_mask | (second_mask << 4);

    /* ModRM offsets 1..5 */
    return (modrm_mask | (modrm_mask >> 1) | (modrm_mask >> 2) |
            (modrm_mask >> 3) | (modrm_mask >> 4)) & 0xffff;
}

int scan_refs(const pe_image *image, long refs[SCAN_REF_LIMIT])
{
    long offset = 0;
    int count = 0;
    for (; offset + 32 <= image->size; offset += 16) {
        uint32_t candidates = candidate_mask(image->data + offset);
        while (candidates) {
            long candidate_offset = offset + __builtin_ctz(candidates);
            scan_ref(image, candidate_offset, refs, &count);
            candidates &= candidates - 1;
        }
    }
    for (; offset + 16 < image->size; offset++)
        scan_ref(image, offset, refs, &count);
    return count;
}
