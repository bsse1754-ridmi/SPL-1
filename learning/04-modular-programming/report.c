#include <stdio.h>
#include "report.h"

void print_bitmap_report(const BlockBitmap *bitmap)
{
    if (bitmap == NULL) return;
    printf("Blocks: %d | Allocated: %zu | Free: %zu\n",
           DEMO_BLOCK_COUNT, bitmap_allocated_count(bitmap),
           (size_t)DEMO_BLOCK_COUNT - bitmap_allocated_count(bitmap));
    printf("Bitmap: ");
    for (size_t i = 0; i < DEMO_BLOCK_COUNT; i++) {
        int used = bitmap_is_allocated(bitmap, i);
        putchar(used == 1 ? '1' : '0');
        if ((i + 1) % 8 == 0 && i + 1 < DEMO_BLOCK_COUNT) putchar(' ');
    }
    putchar('\n');
}
