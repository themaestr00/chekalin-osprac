#include <inc/lib.h>
#include "../doom_jos_edition/doomgeneric.h"

static int
ends_with_wad(const char *s) {
    size_t n;
    if (s == NULL) return 0;
    n = strlen(s);
    if (n < 4) return 0;
    return s[n - 4] == '.' &&
           (s[n - 3] == 'w' || s[n - 3] == 'W') &&
           (s[n - 2] == 'a' || s[n - 2] == 'A') &&
           (s[n - 1] == 'd' || s[n - 1] == 'D');
}

static int
has_iwad_flag(int argc, char **argv) {
    int i;
    for (i = 1; i < argc; i++) {
        if (argv[i] && strcmp(argv[i], "-iwad") == 0) return 1;
    }
    return 0;
}

void
umain(int argc, char **argv) {
    sys_display_change_vga_state(1);

    if (!has_iwad_flag(argc, argv)) {
        if (argc == 1) {
            static char *argv2[] = { "doom", "-iwad", "/doom1.wad", NULL };
            argc = 3;
            argv = argv2;
        } else if (argc == 2 && ends_with_wad(argv[1])) {
            static char *argv2[] = { "doom", "-iwad", NULL, NULL };
            argv2[2] = argv[1];
            argc = 3;
            argv = argv2;
        }
    }

    doomgeneric_Create(argc, argv);
    
    while (1) {
        doomgeneric_Tick();
    }
}

