#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <binary-file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror(argv[1]);
        return EXIT_FAILURE;
    }
    unsigned char bytes[16];
    size_t offset = 0, count;
    while ((count = fread(bytes, 1, sizeof bytes, file)) > 0) {
        printf("%08zx  ", offset);
        for (size_t i = 0; i < sizeof bytes; ++i) {
            if (i < count)
                printf("%02x ", bytes[i]);
            else
                printf("   ");
            if (i == 7)
                putchar(' ');
        }
        printf(" | ");
        for (size_t i = 0; i < count; ++i)
            putchar(isprint((unsigned char)bytes[i]) ? bytes[i] : '.');
        putchar('\n');
        offset += count;
    }
    int ok = !ferror(file);
    if (fclose(file) != 0)
        ok = 0;
    if (!ok) {
        fprintf(stderr, "Failed to read binary file\n");
        return EXIT_FAILURE;
    }
    printf("Total bytes: %zu\n", offset);
    return EXIT_SUCCESS;
}
