#include "bitmap.h"

static int valid_block(size_t block)
{
    return block < DEMO_BLOCK_COUNT;
}

void bitmap_init(BlockBitmap *bitmap)
{
    if (bitmap == NULL) return;
    for (size_t i = 0; i < DEMO_BITMAP_BYTES; i++) bitmap->bits[i] = 0;
    bitmap->allocated = 0;
}

int bitmap_is_allocated(const BlockBitmap *bitmap, size_t block)
{
    if (bitmap == NULL || !valid_block(block)) return -1;
    return (bitmap->bits[block / 8] & (unsigned char)(1u << (block % 8))) != 0;
}

int bitmap_allocate(BlockBitmap *bitmap, size_t block)
{
    if (bitmap == NULL || !valid_block(block)) return -1;
    if (bitmap_is_allocated(bitmap, block)) return 0;
    bitmap->bits[block / 8] |= (unsigned char)(1u << (block % 8));
    bitmap->allocated++;
    return 1;
}

int bitmap_release(BlockBitmap *bitmap, size_t block)
{
    if (bitmap == NULL || !valid_block(block)) return -1;
    if (!bitmap_is_allocated(bitmap, block)) return 0;
    bitmap->bits[block / 8] &= (unsigned char)~(1u << (block % 8));
    bitmap->allocated--;
    return 1;
}

size_t bitmap_allocated_count(const BlockBitmap *bitmap)
{
    return bitmap == NULL ? 0 : bitmap->allocated;
}
