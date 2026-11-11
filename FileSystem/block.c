#include "fs.h"
static int block_data_range(int index)
{
    if (index < FS_DATA_START)
        return 0;
    if (index >= FS_BLOCK_COUNT)
        return 0;
    return 1;
}
static int block_bitmap_byte_valid(unsigned char value)
{
    if (value == 0)
        return 1;
    if (value == 1)
        return 1;
    return 0;
}
static int block_count_in_range(int first, int last, int allocated)
{
    int total = 0;
    int index;
    if (first < 0)
        return -1;
    if (last > FS_BLOCK_COUNT)
        return -1;
    if (first > last)
        return -1;
    for (index = first; index < last; index++)
    {
        if (allocated && fs.bitmap[index] != 0)
            total++;
        if (!allocated && fs.bitmap[index] == 0)
            total++;
    }
    return total;
}
static int block_first_free(void)
{
    int index;
    for (index = FS_DATA_START; index < FS_BLOCK_COUNT; index++)
    {
        if (fs.bitmap[index] == 0)
            return index;
    }
    return FS_NO_SPACE;
}
static int block_write_bitmap_section(int section)
{
    int start;
    int block;
    if (section < 0)
        return FS_INVALID;
    if (section >= FS_INODE_START - FS_BITMAP_START)
        return FS_INVALID;
    start = section * FS_BLOCK_SIZE;
    block = FS_BITMAP_START + section;
    return disk_write_block(block, fs.bitmap + start);
}
static int block_read_bitmap_section(int section)
{
    int start;
    int block;
    if (section < 0)
        return FS_INVALID;
    if (section >= FS_INODE_START - FS_BITMAP_START)
        return FS_INVALID;
    start = section * FS_BLOCK_SIZE;
    block = FS_BITMAP_START + section;
    return disk_read_block(block, fs.bitmap + start);
}
int block_load_bitmap(void)
{
    int section;
    int result;
    for (section = 0; section < FS_INODE_START - FS_BITMAP_START; section++)
    {
        result = block_read_bitmap_section(section);
        if (result != FS_OK)
            return result;
    }
    return FS_OK;
}
int block_save_bitmap(void)
{
    int section;
    int result;
    for (section = 0; section < FS_INODE_START - FS_BITMAP_START; section++)
    {
        result = block_write_bitmap_section(section);
        if (result != FS_OK)
            return result;
    }
    return disk_flush();
}
int block_is_allocated(int index)
{
    if (!disk_is_valid_block(index))
        return FS_INVALID;
    if (fs.bitmap[index] != 0)
        return 1;
    return 0;
}
int block_allocate(void)
{
    int result;
    int index;
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    if (fs.super.free_blocks < 1)
        return FS_NO_SPACE;
    index = block_first_free();
    if (index < 0)
        return index;
    result = disk_clear_block(index);
    if (result != FS_OK)
        return result;
    fs.bitmap[index] = 1;
    fs.super.free_blocks--;
    result = block_save_bitmap();
    if (result != FS_OK)
    {
        fs.bitmap[index] = 0;
        fs.super.free_blocks++;
        return result;
    }
    result = super_save();
    if (result != FS_OK)
        return result;
    return index;
}
int block_release(int index)
{
    int result;
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    if (!block_data_range(index))
        return FS_INVALID;
    if (fs.bitmap[index] != 1)
        return FS_INVALID;
    result = disk_clear_block(index);
    if (result != FS_OK)
        return result;
    fs.bitmap[index] = 0;
    fs.super.free_blocks++;
    result = block_save_bitmap();
    if (result != FS_OK)
        return result;
    return super_save();
}
int block_read_data(int index, void *data)
{
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    if (!block_data_range(index))
        return FS_INVALID;
    if (data == NULL)
        return FS_INVALID;
    if (fs.bitmap[index] != 1)
        return FS_INVALID;
    return disk_read_block(index, data);
}
int block_write_data(int index, const void *data)
{
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    if (!block_data_range(index))
        return FS_INVALID;
    if (data == NULL)
        return FS_INVALID;
    if (fs.bitmap[index] != 1)
        return FS_INVALID;
    return disk_write_block(index, data);
}
int block_count_allocated_data(void)
{
    return block_count_in_range(FS_DATA_START, FS_BLOCK_COUNT, 1);
}
int block_count_free_data(void)
{
    return block_count_in_range(FS_DATA_START, FS_BLOCK_COUNT, 0);
}
int block_verify_bitmap(void)
{
    int i;
    if (block_count_free_data() != fs.super.free_blocks)
        return FS_INVALID;
    for (i = 0; i < FS_BLOCK_COUNT; i++)
    {
        if (!block_bitmap_byte_valid(fs.bitmap[i]))
            return FS_INVALID;
        if (i < FS_DATA_START && fs.bitmap[i] != 1)
            return FS_INVALID;
    }
    return FS_OK;
}
int block_find_first_free(void)
{
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    return block_first_free();
}
int block_count_range(int first, int last, int allocated)
{
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    if (first < FS_DATA_START || last > FS_BLOCK_COUNT)
        return FS_INVALID;
    return block_count_in_range(first, last, allocated != 0);
}
int block_allocate_at(int index)
{
    int result;
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    if (!block_data_range(index))
        return FS_INVALID;
    if (fs.bitmap[index] != 0)
        return FS_EXISTS;
    if (fs.super.free_blocks == 0)
        return FS_NO_SPACE;
    result = disk_clear_block(index);
    if (result != FS_OK)
        return result;
    fs.bitmap[index] = 1;
    fs.super.free_blocks--;
    result = block_save_bitmap();
    if (result != FS_OK)
    {
        fs.bitmap[index] = 0;
        fs.super.free_blocks++;
        return result;
    }
    result = super_save();
    if (result != FS_OK)
    {
        fs.bitmap[index] = 0;
        fs.super.free_blocks++;
        block_save_bitmap();
        return result;
    }
    return FS_OK;
}
static void block_print_row(int first, int last)
{
    int index;
    printf("%4d-%4d  ", first, last - 1);
    for (index = first; index < last; index++)
    {
        if (index < FS_DATA_START)
            putchar('R');
        else if (fs.bitmap[index] != 0)
            putchar('#');
        else
            putchar('.');
        if ((index - first + 1) % 8 == 0)
            putchar(' ');
    }
    putchar('\n');
}
void block_print_bitmap(void)
{
    int first;
    int last;
    int show = 128;
    if (!super_is_ready())
    {
        printf("Disk not formatted.\n");
        return;
    }
    printf("Bitmap: R = reserved, # = used, . = available\n");
    for (first = 0; first < show; first += 32)
    {
        last = first + 32;
        block_print_row(first, last);
    }
    printf("Showing first %d blocks of %d.\n", show, FS_BLOCK_COUNT);
    printf("Data blocks used: %d\n", block_count_allocated_data());
    printf("Data blocks free: %d\n", block_count_free_data());
}
static void block_print_ranges(void)
{
    printf("  block 0       superblock\n");
    printf("  blocks 1-4    allocation bitmap\n");
    printf("  blocks 5-36   reserved for future inode table\n");
    printf("  blocks 37-2047 raw data blocks\n");
}
static void block_print_allocated_ranges(void)
{
    int begin = -1;
    int index;
    int printed = 0;
    for (index = FS_DATA_START; index <= FS_BLOCK_COUNT; index++)
    {
        int used = index < FS_BLOCK_COUNT && fs.bitmap[index] == 1;
        if (used && begin < 0)
            begin = index;
        if (!used && begin >= 0)
        {
            if (begin == index - 1)
                printf("  block %d\n", begin);
            else
                printf("  blocks %d-%d\n", begin, index - 1);
            begin = -1;
            printed++;
        }
    }
    if (!printed)
        printf("  No data blocks currently allocated.\n");
}
void block_print_statistics(void)
{
    int used;
    int free_count;
    int total;
    int percent;
    if (!super_is_ready())
    {
        printf("Storage not formatted.\n");
        return;
    }
    used = block_count_allocated_data();
    free_count = block_count_free_data();
    total = FS_BLOCK_COUNT - FS_DATA_START;
    percent = used * 100 / total;
    printf("Data blocks: %d\n", total);
    printf("Allocated data blocks: %d\n", used);
    printf("Free data blocks: %d\n", free_count);
    printf("Allocation percentage: %d%%\n", percent);
    printf("Allocated bytes: %d\n", used * FS_BLOCK_SIZE);
    printf("Unallocated bytes: %d\n", free_count * FS_BLOCK_SIZE);
    printf("First free data block: %d\n", block_find_first_free());
}
void block_print_map(void)
{
    if (!super_is_ready())
    {
        printf("Disk not formatted.\n");
        return;
    }
    printf("--- Virtual disk map ---\n");
    block_print_ranges();
    printf("--- Allocated data blocks ---\n");
    block_print_allocated_ranges();
    printf("Free data blocks: %d\n", fs.super.free_blocks);
}
