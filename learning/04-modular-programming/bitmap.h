#ifndef LEARNING04_BITMAP_H
#define LEARNING04_BITMAP_H

#include <stddef.h>

#define DEMO_BLOCK_COUNT 32
#define DEMO_BITMAP_BYTES ((DEMO_BLOCK_COUNT + 7) / 8)

typedef struct {
    unsigned char bits[DEMO_BITMAP_BYTES];
    size_t allocated;
} BlockBitmap;

void bitmap_init(BlockBitmap *bitmap);
int bitmap_allocate(BlockBitmap *bitmap, size_t block);
int bitmap_release(BlockBitmap *bitmap, size_t block);
int bitmap_is_allocated(const BlockBitmap *bitmap, size_t block);
size_t bitmap_allocated_count(const BlockBitmap *bitmap);

#endif
