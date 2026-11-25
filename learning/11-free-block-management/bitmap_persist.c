#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE "learning11_bitmap.bin"
static const unsigned char magic[4] = {'B','M','1','1'};

static int save(const BlockBitmap *bitmap)
{
    FILE *f = fopen(IMAGE, "wb");
    if (!f) { perror(IMAGE); return 0; }
    int ok = fwrite(magic, 1, sizeof magic, f) == sizeof magic &&
             fwrite(bitmap->bits, 1, FS_BITMAP_BYTES, f) == FS_BITMAP_BYTES;
    if (fclose(f) != 0) ok = 0;
    if (!ok) fprintf(stderr, "Failed to save bitmap\n");
    return ok;
}

static int load(BlockBitmap *bitmap)
{
    unsigned char header[4];
    FILE *f = fopen(IMAGE, "rb");
    if (!f) { perror(IMAGE); return 0; }
    int ok = fread(header, 1, sizeof header, f) == sizeof header &&
             memcmp(header, magic, sizeof magic) == 0 &&
             fread(bitmap->bits, 1, FS_BITMAP_BYTES, f) == FS_BITMAP_BYTES &&
             fgetc(f) == EOF;
    if (fclose(f) != 0) ok = 0;
    if (ok) {
        for (size_t i = 0; i < FS_RESERVED; ++i)
            if (bitmap_is_used(bitmap, i) != 1) ok = 0;
    }
    if (!ok) fprintf(stderr, "Invalid or incomplete bitmap image\n");
    return ok;
}

int main(int argc, char **argv)
{
    BlockBitmap bitmap;
    if (argc == 2 && strcmp(argv[1], "init") == 0) {
        bitmap_init(&bitmap);
        if (!save(&bitmap)) return 1;
        printf("Initialized %s (%zu free data blocks)\n", IMAGE, bitmap_free_count(&bitmap));
        return 0;
    }
    if (!load(&bitmap)) return 1;
    if (argc == 2 && strcmp(argv[1], "show") == 0) {
        printf("Free data blocks: %zu\nBitmap: ", bitmap_free_count(&bitmap));
        bitmap_print(&bitmap);
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "alloc") == 0) {
        int block = bitmap_allocate(&bitmap);
        if (block < 0) { puts("Disk full"); return 2; }
        if (!save(&bitmap)) return 1;
        printf("Allocated block: %d\n", block);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "free") == 0) {
        char *end;
        unsigned long block = strtoul(argv[2], &end, 10);
        if (!argv[2][0] || *end || block >= FS_BLOCKS) {
            fprintf(stderr, "Invalid block number\n"); return 2;
        }
        int result = bitmap_release(&bitmap, (size_t)block);
        if (result != 1) { fprintf(stderr, "Block reserved or already free\n"); return 2; }
        if (!save(&bitmap)) return 1;
        printf("Freed block: %lu\n", block);
        return 0;
    }
    fprintf(stderr, "Usage: %s init|show|alloc|free BLOCK\n", argv[0]);
    return 2;
}
