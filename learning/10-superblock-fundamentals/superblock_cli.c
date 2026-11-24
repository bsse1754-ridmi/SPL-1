#include "superblock.h"
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <init|verify|inspect|corrupt>\n", argv[0]);
        return 2;
    }
    int ok;
    if (strcmp(argv[1], "init") == 0) ok = sb_create(SB_IMAGE);
    else if (strcmp(argv[1], "verify") == 0) ok = sb_verify(SB_IMAGE);
    else if (strcmp(argv[1], "inspect") == 0) ok = sb_inspect(SB_IMAGE);
    else if (strcmp(argv[1], "corrupt") == 0) ok = sb_corrupt(SB_IMAGE);
    else { fprintf(stderr, "Unknown command: %s\n", argv[1]); return 2; }
    return ok ? 0 : 1;
}
