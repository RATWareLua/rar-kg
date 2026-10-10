#include "derive.h"
#include "sha1.h"
#include <string.h>

/* limb[i] = low16(SHA1(LE32(i+1) || gen[0..4])) -- деривация скаляра из сида */
void derive_limbs(const uint32_t gen[5], uint16_t out[15])
{
    uint32_t message[6], digest[5];
    memcpy(message + 1, gen, 5 * sizeof(*gen));
    for (int i = 0; i < 15; i++) {
        message[0] = (uint32_t)i + 1;
        sha1_words((const uint8_t *)message, sizeof(message), digest);
        out[i] = (uint16_t)digest[0];
    }
}

void pubkey_hex(const point *base, const char *seed, char out[65])
{
    uint32_t seed_hash[5];
    uint16_t limbs[15];
    uint8_t bytes[32];
    sha1_words((const uint8_t *)seed, strlen(seed), seed_hash);
    derive_limbs(seed_hash, limbs);
    point public_key = scalar_mul(base, limbs, 15);
    point_compress(&public_key, bytes);
    for (int i = 0; i < 32; i++) {
        out[2*i] = "0123456789abcdef"[bytes[31-i] >> 4];
        out[2*i+1] = "0123456789abcdef"[bytes[31-i] & 15];
    }
    out[64] = 0;
}
