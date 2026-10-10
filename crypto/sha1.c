#include "sha1.h"
#include <string.h>

static uint32_t rol32(uint32_t x, int n)
{
    return (x << n) | (x >> (32 - n));
}

static void sha1_block(uint32_t h[5], const uint8_t *p)
{
    uint32_t w[80], a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
    for (int i = 0; i < 16; i++)
        w[i] = (uint32_t)p[4*i] << 24 | (uint32_t)p[4*i+1] << 16 |
               (uint32_t)p[4*i+2] << 8 | p[4*i+3];
    for (int i = 16; i < 80; i++)
        w[i] = rol32(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    for (int i = 0; i < 80; i++) {
        uint32_t f, k;
        if (i < 20) {
            f = (b & c) | (~b & d);
            k = 0x5a827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ed9eba1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8f1bbcdc;
        } else {
            f = b ^ c ^ d;
            k = 0xca62c1d6;
        }
        uint32_t t = rol32(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = rol32(b, 30);
        b = a;
        a = t;
    }
    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
    h[4] += e;
}

void sha1_words(const uint8_t *p, size_t n, uint32_t w[5])
{
    uint32_t h[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
    uint64_t bits = (uint64_t)n * 8;
    uint8_t tail[128] = {0};
    while (n >= 64) {
        sha1_block(h, p);
        p += 64;
        n -= 64;
    }
    memcpy(tail, p, n);
    tail[n] = 0x80;
    int size = n < 56 ? 64 : 128;
    for (int i = 0; i < 8; i++)
        tail[size - 1 - i] = (uint8_t)(bits >> (8 * i));
    sha1_block(h, tail);
    if (size == 128)
        sha1_block(h, tail + 64);
    memcpy(w, h, sizeof(h));
}
