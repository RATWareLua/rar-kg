#include "app/console.h"
#include "pe/constants.h"
#include "app/key.h"
#include "app/registration.h"
#include "app/rng.h"
#include "crypto/crc32.h"
#include "crypto/gf15.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    console_options options = {0};
    curve_params curve;
    key_data key;
    chacha20_rng rng;
    int status = 1;
    if (!console_read(argc, argv, &options))
        goto cleanup;
    gf_init();
    crc32_init();
    while (!constants_load(options.exe, &curve)) {
        if (!console_retry_path(&options))
            goto cleanup;
    }
    if (!rng_init(&rng)) {
        puts("Cannot initialize random generator.");
        goto cleanup;
    }
    if (!key_generate(&curve, options.username, options.license_type, &rng, &key)) {
        puts("Cannot generate key.");
        goto cleanup;
    }
    status = !registration_write(options.username, options.license_type, &key);
cleanup:
    console_finish(options.interactive);
    return status;
}
