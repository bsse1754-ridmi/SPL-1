#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define BLOCK_COUNT 16
#define BITS_PER_BYTE 8
#define BITMAP_BYTES ((BLOCK_COUNT + BITS_PER_BYTE - 1) / BITS_PER_BYTE)

/* Convention: 1 = allocated, 0 = free; block 0 is the least significant bit. */
static int valid_block(size_t block)
{
    return block < BLOCK_COUNT;
}

static int block_is_allocated(const uint8_t bitmap[BITMAP_BYTES], size_t block)
{
    if (!valid_block(block)) return -1;
    return (bitmap[block / BITS_PER_BYTE] >> (block % BITS_PER_BYTE)) & 1u;
}

static int allocate_block(uint8_t bitmap[BITMAP_BYTES], size_t block)
{
    if (!valid_block(block) || block_is_allocated(bitmap, block) != 0) return 0;
    bitmap[block / BITS_PER_BYTE] |= (uint8_t)(1u << (block % BITS_PER_BYTE));
    return 1;
}

static int free_block(uint8_t bitmap[BITMAP_BYTES], size_t block)
{
    if (!valid_block(block) || block_is_allocated(bitmap, block) != 1) return 0;
    bitmap[block / BITS_PER_BYTE] &= (uint8_t)~(1u << (block % BITS_PER_BYTE));
    return 1;
}

static void print_bitmap(const uint8_t bitmap[BITMAP_BYTES])
{
    printf("Blocks  0..15 : ");
    for (size_t block = 0; block < BLOCK_COUNT; block++) {
        printf("%d", block_is_allocated(bitmap, block));
        if (block == 7) putchar(' ');
    }
    putchar('\n');
}

static int find_first_free(const uint8_t bitmap[BITMAP_BYTES])
{
    for (size_t block = 0; block < BLOCK_COUNT; block++)
        if (block_is_allocated(bitmap, block) == 0) return (int)block;
    return -1;
}

int main(void)
{
    uint8_t bitmap[BITMAP_BYTES] = {0};
    const unsigned int a = 12u; /* 00001100 */
    const unsigned int b = 10u; /* 00001010 */

    puts("Bitwise operators (a=12, b=10):");
    printf("AND: %u\n", a & b);
    printf("OR : %u\n", a | b);
    printf("XOR: %u\n", a ^ b);
    printf("NOT (low 8 bits of a): %u\n", (~a) & 0xffu);
    printf("a << 1: %u\n", a << 1);
    printf("a >> 1: %u\n", a >> 1);

    puts("\nFile-system block bitmap (1=allocated, 0=free):");
    print_bitmap(bitmap);

    printf("Allocate block 0: %s\n", allocate_block(bitmap, 0) ? "OK" : "FAILED");
    printf("Allocate block 3: %s\n", allocate_block(bitmap, 3) ? "OK" : "FAILED");
    printf("Allocate block 9: %s\n", allocate_block(bitmap, 9) ? "OK" : "FAILED");
    print_bitmap(bitmap);

    printf("Allocate block 3 again: %s\n", allocate_block(bitmap, 3) ? "OK" : "FAILED (already allocated)");
    printf("Free block 3: %s\n", free_block(bitmap, 3) ? "OK" : "FAILED");
    print_bitmap(bitmap);
    printf("First free block: %d\n", find_first_free(bitmap));
    printf("Block 9 allocated? %s\n", block_is_allocated(bitmap, 9) == 1 ? "YES" : "NO");
    printf("Invalid block 16 allocation: %s\n", allocate_block(bitmap, 16) ? "OK" : "REJECTED");
    printf("Bitmap storage: %zu bytes for %d blocks\n", sizeof(bitmap), BLOCK_COUNT);

    return 0;
}
