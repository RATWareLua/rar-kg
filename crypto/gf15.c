#include "gf15.h"

static uint16_t ALOG[0x8000], LOGT[0x8000];

void gf_init(void)
{
    ALOG[0] = 1;
    for (int i = 0; i < 0x7ffe; i++) {
        uint32_t v = (uint32_t)ALOG[i] << 1;
        ALOG[i+1] = (uint16_t)(v & 0x8000 ? v ^ 0x8003 : v);
    }
    for (int i = 0; i < 0x7fff; i++)
        LOGT[ALOG[i]] = (uint16_t)i;
    LOGT[0] = 0x7fff;
}

uint16_t gfmul(uint16_t a, uint16_t b)
{
    if (!a || !b)
        return 0;
    int log_sum = LOGT[a] + LOGT[b];
    if (log_sum >= 0x7fff)
        log_sum -= 0x7fff;
    return ALOG[log_sum];
}
