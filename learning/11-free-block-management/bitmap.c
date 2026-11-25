#include "bitmap.h"
#include <stdio.h>
#include <string.h>

static uint8_t mask_for(size_t block)
{
    return (uint8_t)(1u << (block % 8u));
}

void bitmap_init(BlockBitmap *bitmap)
{
    if (!bitmap) return;
    memset(bitmap->bits, 0, sizeof bitmap->bits);
    for (size_t i = 0; i < FS_RESERVED; ++i)
        bitmap->bits[i / 8u] |= mask_for(i);
}

int bitmap_is_used(const BlockBitmap *bitmap, size_t block)
{
    if (!bitmap || block >= FS_BLOCKS) return -1;
    return (bitmap->bits[block / 8u] & mask_for(block)) != 0;
}

int bitmap_allocate(BlockBitmap *bitmap)
{
    if (!bitmap) return -1;
    for (size_t block = FS_RESERVED; block < FS_BLOCKS; ++block) {
        if (!bitmap_is_used(bitmap, block)) {
            bitmap->bits[block / 8u] |= mask_for(block);
            return (int)block;
        }
    }
    return -1;
}

int bitmap_release(BlockBitmap *bitmap, size_t block)
{
    if (!bitmap || block < FS_RESERVED || block >= FS_BLOCKS) return -1;
    if (!bitmap_is_used(bitmap, block)) return 0;
    bitmap->bits[block / 8u] &= (uint8_t)~mask_for(block);
    return 1;
}

size_t bitmap_free_count(const BlockBitmap *bitmap)
{
    if (!bitmap) return 0;
    size_t count = 0;
    for (size_t b = FS_RESERVED; b < FS_BLOCKS; ++b)
        if (bitmap_is_used(bitmap, b) == 0) ++count;
    return count;
}

void bitmap_print(const BlockBitmap *bitmap)
{
    if (!bitmap) return;
    for (size_t b = 0; b < FS_BLOCKS; ++b) {
        printf("%d", bitmap_is_used(bitmap, b));
        if (b % 8u == 7u) putchar(b + 1u == FS_BLOCKS ? '\n' : ' ');
    }
}
