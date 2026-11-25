#ifndef LEARNING11_BITMAP_H
#define LEARNING11_BITMAP_H
#include <stddef.h>
#include <stdint.h>
#define FS_BLOCKS 64u
#define FS_RESERVED 11u
#define FS_BITMAP_BYTES (FS_BLOCKS / 8u)
typedef struct {
    uint8_t bits[FS_BITMAP_BYTES];
} BlockBitmap;
void bitmap_init(BlockBitmap *bitmap);
int bitmap_is_used(const BlockBitmap *bitmap, size_t block);
int bitmap_allocate(BlockBitmap *bitmap);
int bitmap_release(BlockBitmap *bitmap, size_t block);
size_t bitmap_free_count(const BlockBitmap *bitmap);
void bitmap_print(const BlockBitmap *bitmap);
#endif
