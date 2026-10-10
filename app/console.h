#ifndef KEYGEN_CONSOLE_H
#define KEYGEN_CONSOLE_H

typedef struct {
    int interactive;
    char name[448];
    char license[512];
    char path[512];
    const char *username;
    const char *license_type;
    const char *exe;
} console_options;

int console_read(int argc, char **argv, console_options *options);
int console_retry_path(console_options *options);
void console_finish(int interactive);

#endif
