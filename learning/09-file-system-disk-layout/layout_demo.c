#include <stdio.h>
#include <stddef.h>

#define BLOCK_SIZE 512u
#define TOTAL_BLOCKS 128u
#define SUPER_START 0u
#define SUPER_BLOCKS 1u
#define BLOCK_BITMAP_START 1u
#define BLOCK_BITMAP_BLOCKS 1u
#define INODE_BITMAP_START 2u
#define INODE_BITMAP_BLOCKS 1u
#define INODE_TABLE_START 3u
#define INODE_TABLE_BLOCKS 8u
#define DATA_START (INODE_TABLE_START + INODE_TABLE_BLOCKS)

static void show_region(const char *name, unsigned start, unsigned count)
{
    unsigned end = start + count - 1u;
    printf("%-18s %5u %5u %8u %10u\n", name, start, end,
           start * BLOCK_SIZE, count * BLOCK_SIZE);
}

int main(void)
{
    printf("Learning 09: Virtual File System Disk Layout\n");
    printf("Block size: %u bytes | Total blocks: %u | Capacity: %u bytes\n\n",
           BLOCK_SIZE, TOTAL_BLOCKS, BLOCK_SIZE * TOTAL_BLOCKS);
    puts("Region             First  Last   Offset     Bytes");
    puts("---------------------------------------------------");
    show_region("Superblock", SUPER_START, SUPER_BLOCKS);
    show_region("Block bitmap", BLOCK_BITMAP_START, BLOCK_BITMAP_BLOCKS);
    show_region("Inode bitmap", INODE_BITMAP_START, INODE_BITMAP_BLOCKS);
    show_region("Inode table", INODE_TABLE_START, INODE_TABLE_BLOCKS);
    show_region("Data blocks", DATA_START, TOTAL_BLOCKS - DATA_START);
    printf("\nReserved blocks: %u\nData blocks: %u\n",
           DATA_START, TOTAL_BLOCKS - DATA_START);
    if (DATA_START >= TOTAL_BLOCKS) {
        fputs("Invalid layout\n", stderr);
        return 1;
    }
    return 0;
}
