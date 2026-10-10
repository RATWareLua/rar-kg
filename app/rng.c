#include "rng.h"
#include <windows.h>
#include <bcrypt.h>

int rng_init(chacha20_rng *rng)
{
    struct {
        uint32_t key[8];
        uint32_t nonce[3];
    } seed;

    NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)&seed, sizeof(seed),
                                     BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status < 0) {
        SecureZeroMemory(&seed, sizeof(seed));
        return 0;
    }
    chacha20_init(rng, seed.key, seed.nonce);
    SecureZeroMemory(&seed, sizeof(seed));
    return 1;
}
