#include "signature.h"
#include "bigint.h"
#include "sha1.h"
#include <string.h>

static void hash_e(const uint8_t *msg, size_t len, uint16_t limbs[15])
{
    /* e = SHA1(msg) || 10-байтовый трейлер -> 30 байт (15 лимбов);
       слова 5..7 -- фиксированный трейлер, sha1_words перезаписывает слова 0..4 */
    uint32_t words[8] = {0, 0, 0, 0, 0, 0x0ffd8d43, 0xb4e33c7c, 0x53461bd1};
    sha1_words(msg, len, words);
    memcpy(limbs, words, 30);
}

/* r = (x(kG) + e) mod n; s = (k - d*r) mod n */
int sign_msg(const point *base, const uint16_t order[16], const uint16_t d_limbs[15],
             const char *msg, chacha20_rng *rng, char s_hex[61], char r_hex[61])
{
    uint16_t e_limbs[15];
    hash_e((const uint8_t *)msg, strlen(msg), e_limbs);
    bn modulus = bn_from_u16limbs(order, 16);
    bn private_scalar = bn_from_u16limbs(d_limbs, 15);
    bn message_hash = bn_from_u16limbs(e_limbs, 15);
    for (;;) {
        bn nonce = {0};
        nonce.n = 15;
        if (!chacha20_read(rng, nonce.w, 30))
            return 0;
        bn_norm(&nonce);
        if (!nonce.n)
            continue;

        point nonce_point = scalar_mul(base, nonce.w, 15);
        uint8_t x_bytes[32];
        bn x_coordinate;
        fe_to_bytes(&nonce_point.x, x_bytes);
        bn_from_bytes(&x_coordinate, x_bytes, sizeof(x_bytes));

        bn sum = bn_add(&x_coordinate, &message_hash);
        bn r = bn_mod(&sum, &modulus);
        if (!r.n || r.n > 15)
            continue;

        bn private_product = bn_mulmod(&private_scalar, &r, &modulus);
        if (bn_cmp(&private_product, &nonce) > 0)
            nonce = bn_add(&nonce, &modulus);
        bn s = bn_sub(&nonce, &private_product);
        if (!s.n || s.n > 15)
            continue;

        bn_to_hex(&s, 60, s_hex);
        bn_to_hex(&r, 60, r_hex);
        return 1;
    }
}
