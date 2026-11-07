#include <stdio.h>
#include "bitmap.h"
#include "report.h"

int main(void)
{
    BlockBitmap bitmap;
    bitmap_init(&bitmap);
    printf("Learning 04: Modular C file system demo\n");
    printf("Allocate block 0: %d\n", bitmap_allocate(&bitmap, 0));
    printf("Allocate block 9: %d\n", bitmap_allocate(&bitmap, 9));
    printf("Allocate block 9 again: %d\n", bitmap_allocate(&bitmap, 9));
    printf("Release block 0: %d\n", bitmap_release(&bitmap, 0));
    printf("Allocate invalid block 32: %d\n", bitmap_allocate(&bitmap, 32));
    print_bitmap_report(&bitmap);
    return 0;
}
