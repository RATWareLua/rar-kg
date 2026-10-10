#include "registration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int registration_write(const char *username, const char *license, const key_data *key)
{
    const char *appdata = getenv("APPDATA");
    if (!appdata || !*appdata) {
        puts("APPDATA is not set.");
        return 0;
    }
    char path[512];
    int path_length = snprintf(path, sizeof(path), "%s\\WinRAR\\rarreg.key", appdata);
    if (path_length < 0 || (size_t)path_length >= sizeof(path)) {
        puts("APPDATA path is too long.");
        return 0;
    }
    char *slash = strrchr(path, '\\');
    *slash = 0;
    if (!CreateDirectoryA(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        printf("cannot create %s\n", path);
        return 0;
    }
    *slash = '\\';
    FILE *file = fopen(path, "wb");
    if (!file) {
        printf("cannot write %s\n", path);
        return 0;
    }
    int ok = fprintf(file, "RAR registration data\r\n%s\r\n%s\r\nUID=%s\r\n",
                     username, license, key->uid) >= 0;
    size_t len = strlen(key->data);
    for (size_t i = 0; i < len && ok; i += 54)
        ok = fprintf(file, "%.54s\r\n", key->data + i) >= 0;
    if (fclose(file))
        ok = 0;
    printf(ok ? "written: %s\n" : "cannot write %s\n", path);
    return ok;
}
