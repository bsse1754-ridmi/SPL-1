#include "superblock.h"
#include <stdio.h>
#include <string.h>

static void put32(unsigned char *p, uint32_t x)
{
    for (unsigned i = 0; i < 4; i++) p[i] = (unsigned char)(x >> (8u * i));
}
static uint32_t get32(const unsigned char *p)
{
    uint32_t result = 0;
    for (unsigned i = 0; i < 4; i++) result |= (uint32_t)p[i] << (8u * i);
    return result;
}
static uint32_t checksum(const unsigned char *p)
{
    uint32_t h = UINT32_C(2166136261);
    for (size_t i = 0; i < SB_HEADER_BYTES - 4u; i++) {
        h ^= p[i];
        h *= UINT32_C(16777619);
    }
    return h;
}
static void pack(unsigned char *block)
{
    memset(block, 0, SB_BLOCK_SIZE);
    put32(block + 0, SB_MAGIC);
    put32(block + 4, SB_VERSION);
    put32(block + 8, SB_BLOCK_SIZE);
    put32(block + 12, SB_TOTAL_BLOCKS);
    put32(block + 16, 1u);  /* block bitmap: block 1 */
    put32(block + 20, 2u);  /* inode bitmap: block 2 */
    put32(block + 24, 3u);  /* inode table: blocks 3-10 */
    put32(block + 28, 8u);
    put32(block + 32, SB_DATA_START);
    put32(block + 36, SB_TOTAL_BLOCKS - SB_DATA_START);
    put32(block + 40, 64u); /* simplified inode capacity */
    put32(block + 44, 64u); /* initially free inodes */
    put32(block + 48, SB_TOTAL_BLOCKS - SB_DATA_START); /* free data blocks */
    put32(block + 60, checksum(block));
}
static int read_header(const char *path, unsigned char *b)
{
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return 0; }
    int ok = fread(b, 1, SB_BLOCK_SIZE, f) == SB_BLOCK_SIZE;
    if (ok) {
        if (fseek(f, 0, SEEK_END) != 0 || ftell(f) != (long)(SB_BLOCK_SIZE * SB_TOTAL_BLOCKS)) ok = 0;
    }
    if (fclose(f) != 0) ok = 0;
    if (!ok) fprintf(stderr, "Invalid or truncated disk image\n");
    return ok;
}
static int valid(const unsigned char *b)
{
    uint32_t stored = get32(b + 60);
    uint32_t expected = checksum(b);
    if (stored != expected) { fprintf(stderr, "Checksum mismatch\n"); return 0; }
    if (get32(b + 0) != SB_MAGIC || get32(b + 4) != SB_VERSION ||
        get32(b + 8) != SB_BLOCK_SIZE || get32(b + 12) != SB_TOTAL_BLOCKS ||
        get32(b + 16) != 1u || get32(b + 20) != 2u ||
        get32(b + 24) != 3u || get32(b + 28) != 8u ||
        get32(b + 32) != SB_DATA_START ||
        get32(b + 36) != SB_TOTAL_BLOCKS - SB_DATA_START ||
        get32(b + 40) != 64u || get32(b + 44) > 64u ||
        get32(b + 48) > SB_TOTAL_BLOCKS - SB_DATA_START) {
        fprintf(stderr, "Superblock geometry or metadata invalid\n");
        return 0;
    }
    for (size_t i = SB_HEADER_BYTES; i < SB_BLOCK_SIZE; i++) {
        if (b[i] != 0) { fprintf(stderr, "Unexpected padding data\n"); return 0; }
    }
    return 1;
}
int sb_create(const char *path)
{
    unsigned char header[SB_BLOCK_SIZE];
    unsigned char empty[SB_BLOCK_SIZE] = {0};
    pack(header);
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); return 0; }
    int ok = fwrite(header, 1, sizeof(header), f) == sizeof(header);
    for (unsigned i = 1; ok && i < SB_TOTAL_BLOCKS; i++)
        ok = fwrite(empty, 1, sizeof(empty), f) == sizeof(empty);
    if (fclose(f) != 0) ok = 0;
    if (!ok) { perror("create disk"); return 0; }
    printf("Created %s (%u blocks, %u bytes)\n", path,
           SB_TOTAL_BLOCKS, SB_TOTAL_BLOCKS * SB_BLOCK_SIZE);
    return 1;
}
int sb_verify(const char *path)
{
    unsigned char header[SB_BLOCK_SIZE];
    if (!read_header(path, header) || !valid(header)) return 0;
    puts("Superblock validation: PASS");
    return 1;
}
int sb_inspect(const char *path)
{
    unsigned char header[SB_BLOCK_SIZE];
    if (!read_header(path, header) || !valid(header)) return 0;
    printf("Magic: 0x%08X\n", (unsigned)get32(header));
    printf("Version: %u\n", (unsigned)get32(header + 4));
    printf("Block size: %u bytes\n", (unsigned)get32(header + 8));
    printf("Total blocks: %u\n", (unsigned)get32(header + 12));
    printf("Block bitmap starts: %u\n", (unsigned)get32(header + 16));
    printf("Inode bitmap starts: %u\n", (unsigned)get32(header + 20));
    printf("Inode table: %u blocks starting at %u\n",
           (unsigned)get32(header + 28), (unsigned)get32(header + 24));
    printf("Data starts: %u\n", (unsigned)get32(header + 32));
    printf("Data blocks: %u\n", (unsigned)get32(header + 36));
    printf("Inode capacity: %u\n", (unsigned)get32(header + 40));
    printf("Free inodes: %u\n", (unsigned)get32(header + 44));
    printf("Free data blocks: %u\n", (unsigned)get32(header + 48));
    printf("Checksum: 0x%08X\n", (unsigned)get32(header + 60));
    return 1;
}
int sb_corrupt(const char *path)
{
    FILE *f = fopen(path, "r+b");
    if (!f) { perror(path); return 0; }
    if (fseek(f, 4, SEEK_SET) != 0) { fclose(f); return 0; }
    int value = fgetc(f);
    if (value == EOF || fseek(f, 4, SEEK_SET) != 0 || fputc(value ^ 1, f) == EOF) {
        fclose(f); return 0;
    }
    if (fclose(f) != 0) return 0;
    puts("Modified superblock version byte without updating checksum");
    return 1;
}
