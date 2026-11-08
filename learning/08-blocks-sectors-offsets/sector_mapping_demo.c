#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SECTOR_SIZE 512u
#define SECTORS_PER_BLOCK 8u
#define BLOCK_COUNT 64u

static void describe_block(size_t block) {
    const size_t first_sector = block * SECTORS_PER_BLOCK;
    const size_t byte_offset = first_sector * SECTOR_SIZE;
    printf("Block %zu: sectors %zu..%zu, byte offset %zu\n",
           block, first_sector, first_sector + SECTORS_PER_BLOCK - 1u, byte_offset);
}

int main(void) {
    const size_t logical_block_size = (size_t)SECTOR_SIZE * SECTORS_PER_BLOCK;
    printf("Sector size: %u bytes\n", SECTOR_SIZE);
    printf("Logical block size: %zu bytes\n", logical_block_size);
    printf("Virtual disk size: %zu bytes\n", logical_block_size * BLOCK_COUNT);
    describe_block(0);
    describe_block(3);
    describe_block(63);
    const size_t offset = 12345;
    const size_t block = offset / logical_block_size;
    const size_t within_block = offset % logical_block_size;
    const size_t sector = offset / SECTOR_SIZE;
    const size_t within_sector = offset % SECTOR_SIZE;
    printf("Byte offset %zu maps to block %zu (+%zu), sector %zu (+%zu)\n",
           offset, block, within_block, sector, within_sector);
    if (block >= BLOCK_COUNT) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
