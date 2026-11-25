#ifndef LEARNING12_INODE_BITMAP_H
#define LEARNING12_INODE_BITMAP_H
#include <stddef.h>
#include <stdint.h>
#define INODE_COUNT 64u
#define RESERVED_INODES 2u
#define INODE_BITMAP_BYTES (INODE_COUNT / 8u)
typedef struct {
    uint8_t bits[INODE_BITMAP_BYTES];
    unsigned allocated;
} InodeBitmap;
void inode_bitmap_init(InodeBitmap *map);
int inode_bitmap_test(const InodeBitmap *map, unsigned inode);
int inode_bitmap_allocate(InodeBitmap *map);
int inode_bitmap_release(InodeBitmap *map, unsigned inode);
unsigned inode_bitmap_free_count(const InodeBitmap *map);
int inode_bitmap_validate(const InodeBitmap *map);
int inode_bitmap_save(const InodeBitmap *map, const char *filename);
int inode_bitmap_load(InodeBitmap *map, const char *filename);
#endif
