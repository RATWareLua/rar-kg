#include "constants.h"
#include "scan.h"
#include <string.h>

typedef struct {
    long offsets[64];
    int count;
} candidate_list;

/* структура-кандидат: [u32 кол-во лимбов][кол-во x u32], лимбы <= limit */
static int valid_candidate(const pe_image *image, long offset, int limbs, uint32_t limit)
{
    if (offset + 4 + limbs * 4 > image->size || pe_read32(image, offset) != (uint32_t)limbs)
        return 0;
    for (int i = 0; i < limbs; i++) {
        if (pe_read32(image, offset + 4 + 4 * i) > limit)
            return 0;
    }
    uint32_t top = pe_read32(image, offset + 4 * limbs);
    return limbs != 16 || (top != 0 && top <= 255);   /* у порядка верхний лимб = 1 (~2^240) */
}

static candidate_list collect_candidates(const pe_image *image, const long *refs,
                                         int reference_count, int limbs, uint32_t limit)
{
    candidate_list candidates = {0};
    for (int i = 0; i < reference_count && candidates.count < 64; i++) {
        long offset = refs[i];
        if (!valid_candidate(image, offset, limbs, limit))
            continue;
        int duplicate = 0;
        for (int j = 0; j < candidates.count; j++) {
            if (candidates.offsets[j] == offset) {
                duplicate = 1;
                break;
            }
        }
        if (!duplicate)
            candidates.offsets[candidates.count++] = offset;
    }
    return candidates;
}

static void read_limbs(const pe_image *image, long offset, uint16_t *output, int count)
{
    for (int i = 0; i < count; i++)
        output[i] = (uint16_t)pe_read32(image, offset + 4 + 4 * i);
}

static int is_base_candidate(const point *candidate)
{
    fe y_squared, xy, x_squared, x_cubed, coefficient;
    fe_sqr(&y_squared, &candidate->y);
    fe_mul(&xy, &candidate->x, &candidate->y);
    fe_sqr(&x_squared, &candidate->x);
    fe_mul(&x_cubed, &x_squared, &candidate->x);
    fe_xor(&coefficient, &y_squared, &xy);
    fe_xor(&coefficient, &coefficient, &x_cubed);
    return coefficient.cnt == 1; /* b = y^2 + xy + x^3; у настоящей G b = 0xA1 (одночленный) */
}

static int find_base(const pe_image *image, const candidate_list *candidates, point *base)
{
    for (int x = 0; x < candidates->count; x++) {
        for (int y = 0; y < candidates->count; y++) {
            if (x == y)
                continue;
            point candidate = {0};
            candidate.x.cnt = 17;
            candidate.y.cnt = 17;
            read_limbs(image, candidates->offsets[x], candidate.x.c, 17);
            read_limbs(image, candidates->offsets[y], candidate.y.c, 17);
            fe_norm(&candidate.x);
            fe_norm(&candidate.y);
            if (is_base_candidate(&candidate)) {
                *base = candidate;
                return 1;
            }
        }
    }
    return 0;
}

static int find_order(const pe_image *image, const candidate_list *candidates,
                      const point *base, uint16_t order[16])
{
    for (int i = 0; i < candidates->count; i++) {
        uint16_t limbs[16];
        read_limbs(image, candidates->offsets[i], limbs, 16);
        point multiple = scalar_mul(base, limbs, 16);
        if (point_is_inf(&multiple)) {   /* n*G = бесконечность -> это порядок точки */
            memcpy(order, limbs, 16 * sizeof(*order));
            return 1;
        }
    }
    return 0;
}

int constants_load(const char *path, curve_params *curve)
{
    static long references[SCAN_REF_LIMIT];
    pe_image image;
    if (!pe_open(&image, path))
        return 0;

    int reference_count = scan_refs(&image, references);
    candidate_list coordinates = collect_candidates(&image, references, reference_count, 17, 0x7fff); /* G: 17 x 15-бит */
    candidate_list orders = collect_candidates(&image, references, reference_count, 16, 0xffff);      /* n: 16 x 16-бит */
    int ok = find_base(&image, &coordinates, &curve->base) &&
             find_order(&image, &orders, &curve->base, curve->order);
    pe_close(&image);
    return ok;
}
