#include "fs.h"
static FILE *disk_handle = NULL;
static int disk_last_error = FS_OK;
static long disk_last_offset = 0;
static size_t disk_last_count = 0;
static long disk_read_operations = 0;
static long disk_write_operations = 0;
static long disk_bytes_read = 0;
static long disk_bytes_written = 0;
static int disk_check_range(long offset, size_t count)
{
    long limit = (long)FS_BLOCK_COUNT * FS_BLOCK_SIZE;
    if (offset < 0)
        return FS_INVALID;
    if (count == 0)
        return FS_OK;
    if (offset >= limit)
        return FS_INVALID;
    if (count > (size_t)(limit - offset))
        return FS_INVALID;
    return FS_OK;
}
static int disk_seek(long offset)
{
    if (disk_handle == NULL)
        return FS_IO_ERROR;
    if (fseek(disk_handle, offset, SEEK_SET) != 0)
        return FS_IO_ERROR;
    return FS_OK;
}
static int disk_record_error(int code)
{
    disk_last_error = code;
    return code;
}
static int disk_prepare_open(void)
{
    if (disk_handle != NULL)
        return FS_OK;
    return disk_record_error(FS_IO_ERROR);
}
static int disk_read_exact(long offset, void *buffer, size_t count)
{
    size_t amount;
    if (disk_prepare_open() != FS_OK)
        return FS_IO_ERROR;
    if (buffer == NULL)
        return disk_record_error(FS_INVALID);
    if (disk_check_range(offset, count) != FS_OK)
        return disk_record_error(FS_INVALID);
    if (disk_seek(offset) != FS_OK)
        return disk_record_error(FS_IO_ERROR);
    amount = fread(buffer, 1, count, disk_handle);
    disk_last_offset = offset;
    disk_last_count = count;
    if (amount != count)
        return disk_record_error(FS_IO_ERROR);
    disk_read_operations++;
    disk_bytes_read += (long)count;
    return disk_record_error(FS_OK);
}
static int disk_write_exact(long offset, const void *buffer, size_t count)
{
    size_t amount;
    if (disk_prepare_open() != FS_OK)
        return FS_IO_ERROR;
    if (buffer == NULL)
        return disk_record_error(FS_INVALID);
    if (disk_check_range(offset, count) != FS_OK)
        return disk_record_error(FS_INVALID);
    if (disk_seek(offset) != FS_OK)
        return disk_record_error(FS_IO_ERROR);
    amount = fwrite(buffer, 1, count, disk_handle);
    disk_last_offset = offset;
    disk_last_count = count;
    if (amount != count)
        return disk_record_error(FS_IO_ERROR);
    disk_write_operations++;
    disk_bytes_written += (long)count;
    return disk_record_error(FS_OK);
}
int disk_open(void)
{
    if (disk_handle != NULL)
        return FS_OK;
    disk_handle = fopen(FS_DISK_NAME, "r+b");
    if (disk_handle == NULL)
        disk_handle = fopen(FS_DISK_NAME, "w+b");
    if (disk_handle == NULL)
        return disk_record_error(FS_IO_ERROR);
    disk_last_error = FS_OK;
    disk_last_offset = 0;
    disk_last_count = 0;
    disk_read_operations = 0;
    disk_write_operations = 0;
    disk_bytes_read = 0;
    disk_bytes_written = 0;
    return FS_OK;
}
int disk_close(void)
{
    int result;
    if (disk_handle == NULL)
        return FS_OK;
    result = fflush(disk_handle);
    if (fclose(disk_handle) != 0)
        result = -1;
    disk_handle = NULL;
    if (result != 0)
        return disk_record_error(FS_IO_ERROR);
    return disk_record_error(FS_OK);
}
int disk_is_open(void)
{
    if (disk_handle == NULL)
        return 0;
    return 1;
}
int disk_read_at(long offset, void *buffer, size_t count)
{
    if (count == 0)
        return disk_check_range(offset, count);
    return disk_read_exact(offset, buffer, count);
}
int disk_write_at(long offset, const void *buffer, size_t count)
{
    if (count == 0)
        return disk_check_range(offset, count);
    return disk_write_exact(offset, buffer, count);
}
int disk_is_valid_block(int block)
{
    if (block < 0)
        return 0;
    if (block >= FS_BLOCK_COUNT)
        return 0;
    return 1;
}
int disk_read_block(int block, void *buffer)
{
    long offset;
    if (!disk_is_valid_block(block))
        return disk_record_error(FS_INVALID);
    if (buffer == NULL)
        return disk_record_error(FS_INVALID);
    offset = (long)block * FS_BLOCK_SIZE;
    return disk_read_at(offset, buffer, FS_BLOCK_SIZE);
}
int disk_write_block(int block, const void *buffer)
{
    long offset;
    if (!disk_is_valid_block(block))
        return disk_record_error(FS_INVALID);
    if (buffer == NULL)
        return disk_record_error(FS_INVALID);
    offset = (long)block * FS_BLOCK_SIZE;
    return disk_write_at(offset, buffer, FS_BLOCK_SIZE);
}
int disk_clear_block(int block)
{
    unsigned char zeros[FS_BLOCK_SIZE];
    int i;
    if (!disk_is_valid_block(block))
        return FS_INVALID;
    for (i = 0; i < FS_BLOCK_SIZE; i++)
        zeros[i] = 0;
    return disk_write_block(block, zeros);
}
int disk_clear_all(void)
{
    unsigned char zeros[FS_BLOCK_SIZE];
    int i;
    int block;
    if (!disk_is_open())
        return FS_IO_ERROR;
    for (i = 0; i < FS_BLOCK_SIZE; i++)
        zeros[i] = 0;
    for (block = 0; block < FS_BLOCK_COUNT; block++)
    {
        if (disk_write_block(block, zeros) != FS_OK)
            return FS_IO_ERROR;
    }
    return disk_flush();
}
long disk_size(void)
{
    long current;
    long length;
    if (!disk_is_open())
        return -1;
    current = ftell(disk_handle);
    if (current < 0)
        return -1;
    if (fseek(disk_handle, 0, SEEK_END) != 0)
        return -1;
    length = ftell(disk_handle);
    if (fseek(disk_handle, current, SEEK_SET) != 0)
        return -1;
    return length;
}
int disk_flush(void)
{
    if (!disk_is_open())
        return disk_record_error(FS_IO_ERROR);
    if (fflush(disk_handle) != 0)
        return disk_record_error(FS_IO_ERROR);
    return disk_record_error(FS_OK);
}
int disk_validate_file_size(void)
{
    long actual;
    long expected;
    actual = disk_size();
    expected = (long)FS_BLOCK_COUNT * FS_BLOCK_SIZE;
    if (actual < 0)
        return FS_IO_ERROR;
    if (actual != expected)
        return FS_INVALID;
    return FS_OK;
}
static void disk_print_layout(void)
{
    printf("Disk filename: %s\n", FS_DISK_NAME);
    printf("Disk capacity: %ld bytes\n", (long)FS_BLOCK_COUNT * FS_BLOCK_SIZE);
    printf("Block size: %d bytes\n", FS_BLOCK_SIZE);
    printf("Total blocks: %d\n", FS_BLOCK_COUNT);
    printf("Superblock: 0\n");
    printf("Bitmap blocks: %d-%d\n", FS_BITMAP_START, FS_INODE_START - 1);
    printf("Inode blocks: %d-%d\n", FS_INODE_START, FS_DATA_START - 1);
    printf("Data blocks: %d-%d\n", FS_DATA_START, FS_BLOCK_COUNT - 1);
}
static void disk_print_io_counts(void)
{
    printf("Read operations: %ld\n", disk_read_operations);
    printf("Write operations: %ld\n", disk_write_operations);
    printf("Bytes read: %ld\n", disk_bytes_read);
    printf("Bytes written: %ld\n", disk_bytes_written);
}
static void disk_print_sample_health(void);
void disk_print_diagnostics(void)
{
    printf("--- Disk diagnostics ---\n");
    disk_print_layout();
    printf("Disk open: %s\n", disk_is_open() ? "yes" : "no");
    printf("Actual size: %ld bytes\n", disk_size());
    printf("Last I/O offset: %ld\n", disk_last_offset);
    printf("Last I/O length: %lu\n", (unsigned long)disk_last_count);
    printf("Last error: %d\n", disk_last_error);
    disk_print_io_counts();
    disk_print_sample_health();
}
static int disk_count_nonzero_bytes(int block)
{
    unsigned char data[FS_BLOCK_SIZE];
    int result;
    int index;
    int count = 0;
    result = disk_read_block(block, data);
    if (result != FS_OK)
        return result;
    for (index = 0; index < FS_BLOCK_SIZE; index++)
    {
        if (data[index] != 0)
            count++;
    }
    return count;
}
static int disk_compare_blocks(int left, int right)
{
    unsigned char first[FS_BLOCK_SIZE];
    unsigned char second[FS_BLOCK_SIZE];
    int result;
    int index;
    if (!disk_is_valid_block(left))
        return FS_INVALID;
    if (!disk_is_valid_block(right))
        return FS_INVALID;
    result = disk_read_block(left, first);
    if (result != FS_OK)
        return result;
    result = disk_read_block(right, second);
    if (result != FS_OK)
        return result;
    for (index = 0; index < FS_BLOCK_SIZE; index++)
    {
        if (first[index] != second[index])
            return 0;
    }
    return 1;
}
static int disk_verify_unused_tail(void)
{
    int block;
    int count;
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    for (block = FS_BLOCK_COUNT - 1; block >= FS_DATA_START; block--)
    {
        if (fs.bitmap[block] != 0)
            continue;
        count = disk_count_nonzero_bytes(block);
        if (count < 0)
            return count;
        if (count != 0)
            return FS_INVALID;
        return FS_OK;
    }
    return FS_OK;
}
static void disk_print_sample_health(void)
{
    int count;
    int sample;
    if (!super_is_ready())
        return;
    sample = disk_count_nonzero_bytes(0);
    if (sample < 0)
        printf("Superblock sample: unreadable\n");
    else
        printf("Superblock nonzero bytes: %d\n", sample);
    count = disk_verify_unused_tail();
    printf("Unused tail sample: %s\n", count == FS_OK ? "clear" : "requires review");
    count = disk_compare_blocks(0, 0);
    printf("Read consistency sample: %s\n", count == 1 ? "pass" : "fail");
}
