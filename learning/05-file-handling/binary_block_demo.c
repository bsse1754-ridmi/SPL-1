#include <stdio.h>
#include <string.h>

#define DISK_NAME "learning05_disk.bin"
#define BLOCK_SIZE 64
#define BLOCK_COUNT 8

static int create_disk(void)
{
    FILE *disk = fopen(DISK_NAME, "wb");
    if (disk == NULL) {
        perror("Creating disk failed");
        return 0;
    }
    unsigned char zeros[BLOCK_SIZE] = {0};
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (fwrite(zeros, 1, BLOCK_SIZE, disk) != BLOCK_SIZE) {
            perror("Initializing disk failed");
            fclose(disk);
            return 0;
        }
    }
    if (fclose(disk) == EOF) {
        perror("Closing created disk failed");
        return 0;
    }
    return 1;
}

static int write_block(unsigned block_number, const unsigned char *data)
{
    if (block_number >= BLOCK_COUNT || data == NULL)
        return 0;
    FILE *disk = fopen(DISK_NAME, "rb+");
    if (disk == NULL) {
        perror("Opening disk for writing failed");
        return 0;
    }
    long offset = (long)block_number * BLOCK_SIZE;
    if (fseek(disk, offset, SEEK_SET) != 0 ||
        fwrite(data, 1, BLOCK_SIZE, disk) != BLOCK_SIZE) {
        perror("Writing block failed");
        fclose(disk);
        return 0;
    }
    if (fclose(disk) == EOF) {
        perror("Closing written disk failed");
        return 0;
    }
    return 1;
}

static int read_block(unsigned block_number, unsigned char *data)
{
    if (block_number >= BLOCK_COUNT || data == NULL)
        return 0;
    FILE *disk = fopen(DISK_NAME, "rb");
    if (disk == NULL) {
        perror("Opening disk for reading failed");
        return 0;
    }
    long offset = (long)block_number * BLOCK_SIZE;
    if (fseek(disk, offset, SEEK_SET) != 0 ||
        fread(data, 1, BLOCK_SIZE, disk) != BLOCK_SIZE) {
        perror("Reading block failed");
        fclose(disk);
        return 0;
    }
    if (fclose(disk) == EOF) {
        perror("Closing read disk failed");
        return 0;
    }
    return 1;
}

int main(void)
{
    unsigned char data[BLOCK_SIZE] = {0};
    unsigned char result[BLOCK_SIZE] = {0};
    const char *message = "Block 3 contains file data";
    memcpy(data, message, strlen(message) + 1);

    if (!create_disk()) return 1;
    if (!write_block(3, data)) return 1;
    if (!read_block(3, result)) return 1;
    if (memcmp(data, result, BLOCK_SIZE) != 0) {
        fprintf(stderr, "Block verification failed\n");
        return 1;
    }
    printf("Virtual disk: %s\n", DISK_NAME);
    printf("Total bytes: %d\n", BLOCK_SIZE * BLOCK_COUNT);
    printf("Block 3 offset: %d\n", 3 * BLOCK_SIZE);
    printf("Block 3 data: %s\n", (char *)result);
    printf("Read-back verification: PASS\n");
    if (write_block(BLOCK_COUNT, data)) {
        fprintf(stderr, "Invalid block was accepted\n");
        return 1;
    }
    printf("Out-of-range block rejected: PASS\n");
    return 0;
}
