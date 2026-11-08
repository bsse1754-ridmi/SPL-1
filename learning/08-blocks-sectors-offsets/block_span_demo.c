#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define IMAGE "learning08_disk.bin"
#define BLOCK_SIZE 512u
#define BLOCK_COUNT 16u
#define DISK_SIZE ((size_t)BLOCK_SIZE * BLOCK_COUNT)

static int check_range(size_t offset, size_t count) {
    return offset <= DISK_SIZE && count <= DISK_SIZE - offset;
}

static int create_image(void) {
    FILE *f = fopen(IMAGE, "wb");
    unsigned char zero[BLOCK_SIZE] = {0};
    if (!f) { perror("create image"); return 0; }
    for (size_t i = 0; i < BLOCK_COUNT; i++) {
        if (fwrite(zero, 1, sizeof zero, f) != sizeof zero) {
            perror("initialize image"); fclose(f); return 0;
        }
    }
    if (fclose(f) != 0) { perror("close image"); return 0; }
    return 1;
}

static int image_size_valid(FILE *f) {
    if (fseek(f, 0, SEEK_END) != 0) return 0;
    long bytes = ftell(f);
    return bytes >= 0 && (size_t)bytes == DISK_SIZE;
}

static int write_bytes(size_t offset, const unsigned char *data, size_t count) {
    if (!check_range(offset, count)) return 0;
    FILE *f = fopen(IMAGE, "r+b");
    if (!f) { perror("write open"); return 0; }
    int ok = image_size_valid(f) && fseek(f, (long)offset, SEEK_SET) == 0;
    if (ok && count) ok = fwrite(data, 1, count, f) == count;
    if (fflush(f) != 0) ok = 0;
    if (fclose(f) != 0) ok = 0;
    return ok;
}

static int read_bytes(size_t offset, unsigned char *data, size_t count) {
    if (!check_range(offset, count)) return 0;
    FILE *f = fopen(IMAGE, "rb");
    if (!f) { perror("read open"); return 0; }
    int ok = image_size_valid(f) && fseek(f, (long)offset, SEEK_SET) == 0;
    if (ok && count) ok = fread(data, 1, count, f) == count;
    if (fclose(f) != 0) ok = 0;
    return ok;
}

static void print_mapping(size_t offset, size_t count) {
    printf("Byte range: [%zu, %zu)\n", offset, offset + count);
    if (count == 0) { puts("No blocks touched"); return; }
    size_t first = offset / BLOCK_SIZE;
    size_t last = (offset + count - 1) / BLOCK_SIZE;
    printf("First block: %zu, first in-block offset: %zu\n", first, offset % BLOCK_SIZE);
    printf("Last block: %zu, blocks touched: %zu\n", last, last - first + 1);
}

int main(void) {
    const size_t offset = BLOCK_SIZE - 4u;
    const unsigned char payload[] = "BOUNDARY-CROSSING-DATA";
    unsigned char recovered[sizeof payload];
    if (!create_image()) return EXIT_FAILURE;
    printf("Disk capacity: %zu bytes (%u blocks x %u bytes)\n", DISK_SIZE, BLOCK_COUNT, BLOCK_SIZE);
    print_mapping(offset, sizeof payload);
    if (!write_bytes(offset, payload, sizeof payload)) {
        fputs("Write failed\n", stderr); return EXIT_FAILURE;
    }
    memset(recovered, 0, sizeof recovered);
    if (!read_bytes(offset, recovered, sizeof recovered)) {
        fputs("Read failed\n", stderr); return EXIT_FAILURE;
    }
    printf("Read-back: %s\n", recovered);
    if (memcmp(payload, recovered, sizeof payload) != 0) {
        fputs("Data mismatch\n", stderr); return EXIT_FAILURE;
    }
    puts("Cross-block verification: PASS");
    if (write_bytes(DISK_SIZE - 3u, payload, sizeof payload)) {
        fputs("Out-of-range write was incorrectly accepted\n", stderr);
        return EXIT_FAILURE;
    }
    puts("Out-of-range write rejected: PASS");
    if (read_bytes(DISK_SIZE, recovered, 1)) {
        fputs("Out-of-range read was incorrectly accepted\n", stderr);
        return EXIT_FAILURE;
    }
    puts("Out-of-range read rejected: PASS");
    return EXIT_SUCCESS;
}
