#ifndef FS_H
#define FS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#define FS_BLOCK_SIZE 512
#define FS_BLOCK_COUNT 2048
#define FS_INODE_COUNT 128
#define FS_BITMAP_START 1
#define FS_INODE_START 5
#define FS_DATA_START 37
#define FS_DIRECT_BLOCKS 12
#define FS_NAME_LIMIT 47
#define FS_PATH_LIMIT 512
#define FS_MAGIC 0x46533131
#define FS_VERSION 1
#define FS_DISK_NAME "virtual_disk.bin"
#define FS_OK 0
#define FS_ERROR -1
#define FS_NOT_FOUND -2
#define FS_EXISTS -3
#define FS_NO_SPACE -4
#define FS_INVALID -5
#define FS_NOT_DIRECTORY -6
#define FS_IS_DIRECTORY -7
#define FS_NOT_EMPTY -8
#define FS_NOT_MOUNTED -9
#define FS_TOO_LARGE -10
#define FS_IO_ERROR -11
#define FS_BUSY -12
typedef struct {
    int magic;
    int version;
    int block_size;
    int total_blocks;
    int inode_count;
    int data_start;
    int free_blocks;
    int free_inodes;
    int root_inode;
    int serial;
    unsigned char padding[472];
} FSSuper;
typedef struct {
    FSSuper super;
    unsigned char bitmap[FS_BLOCK_COUNT];
    int mounted;
} FSContext;
extern FSContext fs;
int disk_open(void);
int disk_close(void);
int disk_is_open(void);
int disk_read_at(long offset, void *buffer, size_t count);
int disk_write_at(long offset, const void *buffer, size_t count);
int disk_read_block(int block, void *buffer);
int disk_write_block(int block, const void *buffer);
int disk_clear_block(int block);
int disk_clear_all(void);
long disk_size(void);
int disk_flush(void);
int disk_is_valid_block(int block);
int disk_validate_file_size(void);
void disk_print_diagnostics(void);
int super_format(void);
int super_mount(void);
int super_save(void);
int super_validate(const FSSuper *value);
int super_is_ready(void);
int super_check_consistency(void);
void super_print_info(void);
void super_print_status(void);
int block_load_bitmap(void);
int block_save_bitmap(void);
int block_allocate(void);
int block_allocate_at(int index);
int block_release(int index);
int block_is_allocated(int index);
int block_read_data(int index, void *data);
int block_write_data(int index, const void *data);
int block_count_allocated_data(void);
int block_count_free_data(void);
int block_verify_bitmap(void);
int block_find_first_free(void);
int block_count_range(int first, int last, int allocated);
void block_print_bitmap(void);
void block_print_map(void);
void block_print_statistics(void);
int raw_write_text(int index, const char *text);
int raw_read_text(int index);
int raw_fill(int index, unsigned char value);
int raw_copy(int source, int destination);
int raw_compare(int left, int right);
int raw_zero(int index);
int raw_show_hex(int index, int count);
int raw_show_text(int index, int count);
int raw_count_value(int index, unsigned char value);
int raw_checksum(int index, unsigned long *output);
int raw_inspect(int index);
int raw_write_pattern(int index, const char *pattern);
int raw_find_text(int index, const char *needle);
#endif
