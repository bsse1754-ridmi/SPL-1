#include "fs.h"
FSContext fs;
static int super_check_types(void)
{
    if (sizeof(int) != 4)
        return FS_INVALID;
    if (sizeof(FSSuper) != FS_BLOCK_SIZE)
        return FS_INVALID;
    if ((FS_INODE_START - FS_BITMAP_START) * FS_BLOCK_SIZE < FS_BLOCK_COUNT)
        return FS_INVALID;
    if (FS_DATA_START <= FS_INODE_START)
        return FS_INVALID;
    return FS_OK;
}
static void super_defaults(FSSuper *value)
{
    memset(value, 0, sizeof(*value));
    value->magic = FS_MAGIC;
    value->version = FS_VERSION;
    value->block_size = FS_BLOCK_SIZE;
    value->total_blocks = FS_BLOCK_COUNT;
    value->inode_count = FS_INODE_COUNT;
    value->data_start = FS_DATA_START;
    value->free_blocks = FS_BLOCK_COUNT - FS_DATA_START;
    value->free_inodes = FS_INODE_COUNT;
    value->root_inode = 0;
    value->serial = 1;
}
static void super_reset(void)
{
    memset(&fs, 0, sizeof(fs));
}
static void super_initialize_bitmap(void)
{
    int i;
    for (i = 0; i < FS_BLOCK_COUNT; i++)
        fs.bitmap[i] = 0;
    for (i = 0; i < FS_DATA_START; i++)
        fs.bitmap[i] = 1;
}
int super_validate(const FSSuper *value)
{
    if (value == NULL)
        return FS_INVALID;
    if (value->magic != FS_MAGIC)
        return FS_INVALID;
    if (value->version != FS_VERSION)
        return FS_INVALID;
    if (value->block_size != FS_BLOCK_SIZE)
        return FS_INVALID;
    if (value->total_blocks != FS_BLOCK_COUNT)
        return FS_INVALID;
    if (value->inode_count != FS_INODE_COUNT)
        return FS_INVALID;
    if (value->data_start != FS_DATA_START)
        return FS_INVALID;
    if (value->root_inode != 0)
        return FS_INVALID;
    if (value->free_blocks < 0 || value->free_blocks > FS_BLOCK_COUNT - FS_DATA_START)
        return FS_INVALID;
    if (value->free_inodes != FS_INODE_COUNT)
        return FS_INVALID;
    if (value->serial != 1)
        return FS_INVALID;
    return FS_OK;
}
int super_is_ready(void)
{
    if (fs.mounted == 0)
        return 0;
    if (super_validate(&fs.super) != FS_OK)
        return 0;
    return 1;
}
int super_save(void)
{
    int result;
    if (!disk_is_open())
        return FS_IO_ERROR;
    if (super_validate(&fs.super) != FS_OK)
        return FS_INVALID;
    result = disk_write_block(0, &fs.super);
    if (result != FS_OK)
        return result;
    return disk_flush();
}
int super_format(void)
{
    int result;
    if (!disk_is_open())
        return FS_IO_ERROR;
    if (super_check_types() != FS_OK)
        return FS_INVALID;
    result = disk_clear_all();
    if (result != FS_OK)
        return result;
    super_reset();
    super_defaults(&fs.super);
    super_initialize_bitmap();
    result = block_save_bitmap();
    if (result != FS_OK)
        return result;
    result = super_save();
    if (result != FS_OK)
        return result;
    fs.mounted = 1;
    return super_check_consistency();
}
int super_mount(void)
{
    FSSuper stored;
    int result;
    if (!disk_is_open())
        return FS_IO_ERROR;
    if (super_check_types() != FS_OK)
        return FS_INVALID;
    if (disk_validate_file_size() != FS_OK)
        return FS_INVALID;
    result = disk_read_block(0, &stored);
    if (result != FS_OK)
        return result;
    if (super_validate(&stored) != FS_OK)
        return FS_INVALID;
    super_reset();
    fs.super = stored;
    result = block_load_bitmap();
    if (result != FS_OK)
        return result;
    result = super_check_consistency();
    if (result != FS_OK)
    {
        super_reset();
        return result;
    }
    fs.mounted = 1;
    return FS_OK;
}
static int super_check_reserved_bitmap(void)
{
    int i;
    for (i = 0; i < FS_DATA_START; i++)
    {
        if (fs.bitmap[i] != 1)
            return FS_INVALID;
    }
    return FS_OK;
}
static int super_check_usable_bitmap(void)
{
    int i;
    for (i = FS_DATA_START; i < FS_BLOCK_COUNT; i++)
    {
        if (fs.bitmap[i] != 0 && fs.bitmap[i] != 1)
            return FS_INVALID;
    }
    return FS_OK;
}
static int super_count_available(void)
{
    int count = 0;
    int i;
    for (i = FS_DATA_START; i < FS_BLOCK_COUNT; i++)
    {
        if (fs.bitmap[i] == 0)
            count++;
    }
    return count;
}
static int super_check_reserved_disk_area(void)
{
    unsigned char data[FS_BLOCK_SIZE];
    int i;
    int j;
    for (i = FS_INODE_START; i < FS_DATA_START; i++)
    {
        if (disk_read_block(i, data) != FS_OK)
            return FS_IO_ERROR;
        for (j = 0; j < FS_BLOCK_SIZE; j++)
        {
            if (data[j] != 0)
                return FS_INVALID;
        }
    }
    return FS_OK;
}
int super_check_consistency(void)
{
    if (super_validate(&fs.super) != FS_OK)
        return FS_INVALID;
    if (!disk_is_open())
        return FS_IO_ERROR;
    if (disk_validate_file_size() != FS_OK)
        return FS_INVALID;
    if (super_check_reserved_bitmap() != FS_OK)
        return FS_INVALID;
    if (super_check_usable_bitmap() != FS_OK)
        return FS_INVALID;
    if (super_count_available() != fs.super.free_blocks)
        return FS_INVALID;
    if (super_check_reserved_disk_area() != FS_OK)
        return FS_INVALID;
    return FS_OK;
}
static void super_print_layout(void)
{
    printf("Superblock: block 0\n");
    printf("Bitmap: blocks %d-%d\n", FS_BITMAP_START, FS_INODE_START - 1);
    printf("Reserved for future inode metadata: blocks %d-%d\n", FS_INODE_START, FS_DATA_START - 1);
    printf("User data: blocks %d-%d\n", FS_DATA_START, FS_BLOCK_COUNT - 1);
}
static void super_print_capacity(void)
{
    int data_blocks = FS_BLOCK_COUNT - FS_DATA_START;
    int used = data_blocks - fs.super.free_blocks;
    printf("Disk capacity: %d bytes\n", FS_BLOCK_COUNT * FS_BLOCK_SIZE);
    printf("Data capacity: %d bytes\n", data_blocks * FS_BLOCK_SIZE);
    printf("Data allocated: %d bytes\n", used * FS_BLOCK_SIZE);
    printf("Data available: %d bytes\n", fs.super.free_blocks * FS_BLOCK_SIZE);
    printf("Free data blocks: %d\n", fs.super.free_blocks);
}
static void super_print_format(void)
{
    printf("Format version: %d\n", fs.super.version);
    printf("Magic number: %08X\n", (unsigned int)fs.super.magic);
    printf("Block size: %d bytes\n", fs.super.block_size);
    printf("Total blocks: %d\n", fs.super.total_blocks);
    printf("Inode region: reserved, not implemented\n");
    printf("Future inode capacity: %d\n", fs.super.inode_count);
    printf("Future inode slots currently used: 0\n");
}
void super_print_info(void)
{
    if (!super_is_ready())
    {
        printf("Storage is not formatted.\n");
        return;
    }
    printf("--- Storage information ---\n");
    super_print_format();
    super_print_capacity();
    super_print_layout();
}
void super_print_status(void)
{
    int result;
    if (!super_is_ready())
    {
        printf("Storage status: not mounted\n");
        return;
    }
    result = super_check_consistency();
    printf("Storage status: mounted\n");
    printf("Metadata consistency: %s\n", result == FS_OK ? "PASS" : "FAIL");
    printf("Disk file: %s\n", FS_DISK_NAME);
    printf("Disk length: %ld\n", disk_size());
    printf("Allocated data blocks: %d\n", FS_BLOCK_COUNT - FS_DATA_START - fs.super.free_blocks);
}
