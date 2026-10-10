#include "key.h"
#include "../crypto/crc32.h"
#include "../crypto/derive.h"
#include "../crypto/signature.h"
#include <stdio.h>

typedef struct {
    char public_key[65];
    char derived_key[65];
    char seed[51];
    char license_signature[123];
    char name_signature[123];
} key_parts;

static void make_identity(const point *base, const char *username, key_parts *parts)
{
    pubkey_hex(base, username, parts->public_key);
    snprintf(parts->seed, sizeof(parts->seed), "60%.48s", parts->public_key);
    pubkey_hex(base, parts->seed, parts->derived_key);
}

static int make_signature(const curve_params *curve, const uint16_t private_key[15],
                          const char *message, chacha20_rng *rng, char output[123])
{
    char signature_s[61];
    char signature_r[61];
    if (!sign_msg(&curve->base, curve->order, private_key, message, rng,
                  signature_s, signature_r))
        return 0;
    return snprintf(output, 123, "60%s%s", signature_s, signature_r) == 122;
}

static uint32_t key_checksum(const char *username, const char *license, const key_parts *parts)
{
    const char *strings[] = {
        license, username, parts->derived_key, parts->license_signature,
        parts->name_signature, parts->seed
    };
    uint32_t checksum = 0xffffffffu;
    for (int i = 0; i < 6; i++)
        checksum = crc32_string(checksum, strings[i]);
    return checksum;
}

int key_generate(const curve_params *curve, const char *username, const char *license,
                 chacha20_rng *rng, key_data *key)
{
    /* мастер-сид скаляра d: в winrar.exe не хранится;
       derive_limbs(master_seed) даёт d, согласованный с вшитым публичным ключом Q
       (compress(d*G) == Q) */
    static const uint32_t master_seed[5] = {
        0xeb3eb781, 0x50265329, 0xdc5ef4a3, 0x6847b9d5, 0xcde43b4c
    };
    uint16_t private_key[15];
    char signed_name[512];
    key_parts parts;

    make_identity(&curve->base, username, &parts);
    int name_length = snprintf(signed_name, sizeof(signed_name), "%s%s",
                               username, parts.derived_key);
    if (name_length < 0 || (size_t)name_length >= sizeof(signed_name))
        return 0;

    derive_limbs(master_seed, private_key);
    if (!make_signature(curve, private_key, license, rng, parts.license_signature))
        return 0;
    if (!make_signature(curve, private_key, signed_name, rng, parts.name_signature))
        return 0;

    snprintf(key->uid, sizeof(key->uid), "%.16s%.4s", parts.public_key + 48, parts.derived_key);
    uint32_t checksum = key_checksum(username, license, &parts);
    return snprintf(key->data, sizeof(key->data), "6412212250%s%s%s%s%010u",
                    parts.derived_key, parts.license_signature, parts.name_signature, parts.seed,
                    (unsigned)checksum) == 378;
}
