#include "console.h"
#include <stdio.h>
#include <string.h>

static int prompt(const char *label, char *out, int size)
{
    printf("%s: ", label);
    fflush(stdout);
    if (!fgets(out, size, stdin))
        return 0;

    size_t line_length = strcspn(out, "\r\n");
    if (!out[line_length]) {
        int next_character = getchar();
        if (next_character != '\n' && next_character != EOF) {
            while (next_character != '\n' && next_character != EOF)
                next_character = getchar();
            puts("Input too long.");
            return 0;
        }
    }
    out[line_length] = 0;
    return 1;
}

int console_read(int argc, char **argv, console_options *options)
{
    options->interactive = argc == 1;
    options->exe = argc > 3 ? argv[3] : "C:\\Program Files\\WinRAR\\WinRAR.exe";
    if (options->interactive) {
        puts("\n  WinRAR key generator\n  --------------------\n");
        if (!prompt("Name", options->name, sizeof(options->name)) ||
            !prompt("License type", options->license, sizeof(options->license)))
            return 0;
        options->username = options->name;
        options->license_type = options->license;
    } else {
        if (argc < 3) {
            puts("usage: keygen \"Name\" \"License type\" [winrar.exe]");
            return 0;
        }
        options->username = argv[1];
        options->license_type = argv[2];
    }
    return 1;
}

int console_retry_path(console_options *options)
{
    printf("cannot read %s\n", options->exe);
    if (!prompt("Path to WinRAR.exe (empty = cancel)", options->path, sizeof(options->path)))
        return 0;
    options->exe = options->path;
    size_t path_length = strlen(options->path);
    if (path_length > 1 && options->path[0] == '"' && options->path[path_length - 1] == '"') {
        options->path[path_length - 1] = 0;
        options->exe++;
    }
    return *options->exe != 0;
}

void console_finish(int interactive)
{
    char ignored[512];
    if (interactive)
        prompt("\nPress Enter to exit", ignored, sizeof(ignored));
}
