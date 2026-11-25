#include "bitmap.h"
#include <stdio.h>

int main(void)
{
    BlockBitmap bitmap;
    bitmap_init(&bitmap);
    printf("Total blocks: %u | Reserved: %u | Free: %zu\n",
           FS_BLOCKS, FS_RESERVED, bitmap_free_count(&bitmap));
    printf("First allocation: %d\n", bitmap_allocate(&bitmap));
    printf("Second allocation: %d\n", bitmap_allocate(&bitmap));
    printf("Release block 11: %d\n", bitmap_release(&bitmap, 11));
    printf("Release reserved block 0: %d (rejected)\n", bitmap_release(&bitmap, 0));
    printf("Next allocation (reused): %d\n", bitmap_allocate(&bitmap));
    int allocated = 0;
    while (bitmap_allocate(&bitmap) >= 0) ++allocated;
    printf("Further allocated blocks: %d\n", allocated);
    printf("Allocate when full: %d\n", bitmap_allocate(&bitmap));
    printf("Free data blocks: %zu\n", bitmap_free_count(&bitmap));
    printf("Bitmap: ");
    bitmap_print(&bitmap);
    return bitmap_free_count(&bitmap) == 0 ? 0 : 1;
}
