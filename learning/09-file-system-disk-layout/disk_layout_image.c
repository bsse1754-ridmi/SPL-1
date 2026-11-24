#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define IMAGE "learning09_disk.bin"
#define BLOCK_SIZE 512u
#define TOTAL_BLOCKS 128u
#define DATA_START 11u
#define MAGIC 0x53464C39u

/* Educational on-disk format: 32-bit little-endian fields. */
static void put32(unsigned char *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i) p[i] = (unsigned char)(v >> (8u * i));
}
static uint32_t get32(const unsigned char *p)
{
    uint32_t v = 0;
    for (unsigned i = 0; i < 4; ++i) v |= (uint32_t)p[i] << (8u * i);
    return v;
}
static int write_block(FILE *f, unsigned block, const unsigned char *bytes)
{
    if (block >= TOTAL_BLOCKS || fseek(f, (long)block * BLOCK_SIZE, SEEK_SET) != 0)
        return 0;
    return fwrite(bytes, 1, BLOCK_SIZE, f) == BLOCK_SIZE;
}
static int read_block(FILE *f, unsigned block, unsigned char *bytes)
{
    if (block >= TOTAL_BLOCKS || fseek(f, (long)block * BLOCK_SIZE, SEEK_SET) != 0)
        return 0;
    return fread(bytes, 1, BLOCK_SIZE, f) == BLOCK_SIZE;
}
static int create_image(void)
{
    FILE *f = fopen(IMAGE, "wb+");
    unsigned char block[BLOCK_SIZE] = {0};
    if (!f) { perror(IMAGE); return 0; }
    for (unsigned i = 0; i < TOTAL_BLOCKS; ++i) {
        if (fwrite(block, 1, BLOCK_SIZE, f) != BLOCK_SIZE) {
            perror("initialize"); fclose(f); return 0;
        }
    }
    put32(block + 0, MAGIC);
    put32(block + 4, BLOCK_SIZE);
    put32(block + 8, TOTAL_BLOCKS);
    put32(block + 12, 1u);   /* block bitmap */
    put32(block + 16, 2u);   /* inode bitmap */
    put32(block + 20, 3u);   /* inode table */
    put32(block + 24, DATA_START);
    if (!write_block(f, 0, block)) { fclose(f); return 0; }
    memset(block, 0, sizeof(block));
    /* One bit per disk block. Reserve all metadata blocks [0, DATA_START). */
    for (unsigned i = 0; i < DATA_START; ++i)
        block[i / 8u] |= (unsigned char)(1u << (i % 8u));
    if (!write_block(f, 1, block)) { fclose(f); return 0; }
    memset(block, 0, sizeof(block));
    /* Inode bitmap is initially empty; inode table blocks are zero-filled. */
    if (!write_block(f, 2, block)) { fclose(f); return 0; }
    if (fflush(f) != 0 || fclose(f) != 0) { perror("close"); return 0; }
    puts("Disk image created: " IMAGE);
    return 1;
}
static int inspect_image(void)
{
    FILE *f = fopen(IMAGE, "rb");
    unsigned char sb[BLOCK_SIZE], bitmap[BLOCK_SIZE];
    if (!f) { perror(IMAGE); return 0; }
    if (fseek(f, 0, SEEK_END) != 0 || ftell(f) != (long)(TOTAL_BLOCKS * BLOCK_SIZE) ||
        !read_block(f, 0, sb) || !read_block(f, 1, bitmap)) {
        fputs("Image size or read failed\n", stderr); fclose(f); return 0;
    }
    if (get32(sb) != MAGIC || get32(sb + 4) != BLOCK_SIZE ||
        get32(sb + 8) != TOTAL_BLOCKS || get32(sb + 12) != 1u ||
        get32(sb + 16) != 2u || get32(sb + 20) != 3u ||
        get32(sb + 24) != DATA_START) {
        fputs("Superblock validation failed\n", stderr); fclose(f); return 0;
    }
    for (unsigned i = 0; i < TOTAL_BLOCKS; ++i) {
        unsigned allocated = (bitmap[i / 8u] >> (i % 8u)) & 1u;
        if (allocated != (unsigned)(i < DATA_START)) {
            fputs("Bitmap validation failed\n", stderr); fclose(f); return 0;
        }
    }
    fclose(f);
    puts("Superblock validation: PASS");
    puts("Reserved block bitmap: PASS");
    printf("Metadata blocks: %u | Free data blocks: %u\n",
           DATA_START, TOTAL_BLOCKS - DATA_START);
    return 1;
}
int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <create|inspect>\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "create") == 0) return create_image() ? 0 : 1;
    if (strcmp(argv[1], "inspect") == 0) return inspect_image() ? 0 : 1;
    fputs("Unknown command\n", stderr);
    return 1;
}
