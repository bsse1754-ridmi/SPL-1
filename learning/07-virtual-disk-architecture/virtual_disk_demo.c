#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DISK_FILE "learning07_disk.bin"
#define BLOCK_SIZE 512
#define BLOCK_COUNT 64
#define DISK_BYTES (BLOCK_SIZE * BLOCK_COUNT)

static int parse_block(const char *s, size_t *block) {
    char *end;
    unsigned long number;
    if (!s || !*s || *s == '-') return 0;
    number = strtoul(s, &end, 10);
    if (*end != '\0' || number >= BLOCK_COUNT) return 0;
    *block = (size_t)number;
    return 1;
}

static FILE *open_disk(const char *mode) {
    FILE *file = fopen(DISK_FILE, mode);
    if (!file) perror(DISK_FILE);
    return file;
}

static int init_disk(void) {
    FILE *file = open_disk("wb");
    unsigned char zero[BLOCK_SIZE] = {0};
    if (!file) return 1;
    for (size_t i = 0; i < BLOCK_COUNT; i++) {
        if (fwrite(zero, 1, sizeof(zero), file) != sizeof(zero)) {
            fprintf(stderr, "Disk initialization failed at block %zu\n", i);
            fclose(file);
            return 1;
        }
    }
    if (fclose(file) != 0) {
        perror("close");
        return 1;
    }
    printf("Created %s: %d blocks x %d bytes = %d bytes\n",
           DISK_FILE, BLOCK_COUNT, BLOCK_SIZE, DISK_BYTES);
    return 0;
}

static int validate_disk(FILE *file) {
    long length;
    if (fseek(file, 0, SEEK_END) != 0) return 0;
    length = ftell(file);
    if (length != DISK_BYTES) {
        fprintf(stderr, "Disk image is invalid or has the wrong size\n");
        return 0;
    }
    return 1;
}

static int read_block(FILE *file, size_t block, unsigned char out[BLOCK_SIZE]) {
    if (!validate_disk(file)) return 0;
    if (fseek(file, (long)(block * BLOCK_SIZE), SEEK_SET) != 0) return 0;
    return fread(out, 1, BLOCK_SIZE, file) == BLOCK_SIZE;
}

static int write_block(FILE *file, size_t block, const unsigned char in[BLOCK_SIZE]) {
    if (!validate_disk(file)) return 0;
    if (fseek(file, (long)(block * BLOCK_SIZE), SEEK_SET) != 0) return 0;
    if (fwrite(in, 1, BLOCK_SIZE, file) != BLOCK_SIZE) return 0;
    return fflush(file) == 0;
}

static int info_disk(void) {
    FILE *file = open_disk("rb");
    if (!file) return 1;
    if (!validate_disk(file)) { fclose(file); return 1; }
    printf("Disk: %s\nBlock size: %d bytes\nBlocks: %d\nCapacity: %d bytes\n",
           DISK_FILE, BLOCK_SIZE, BLOCK_COUNT, DISK_BYTES);
    return fclose(file) == 0 ? 0 : 1;
}

static int display_block(size_t block) {
    unsigned char bytes[BLOCK_SIZE];
    FILE *file = open_disk("rb");
    if (!file) return 1;
    int ok = read_block(file, block, bytes);
    if (fclose(file) != 0) ok = 0;
    if (!ok) { fprintf(stderr, "Could not read block %zu\n", block); return 1; }
    printf("Block %zu, offset %zu\n", block, block * (size_t)BLOCK_SIZE);
    printf("Text: ");
    for (size_t i = 0; i < BLOCK_SIZE && bytes[i]; i++) {
        putchar(bytes[i] >= 32 && bytes[i] <= 126 ? bytes[i] : '.');
    }
    putchar('\n');
    printf("First 16 bytes:");
    for (size_t i = 0; i < 16; i++) printf(" %02X", (unsigned int)bytes[i]);
    putchar('\n');
    return 0;
}

static int update_block(size_t block, const char *text) {
    unsigned char bytes[BLOCK_SIZE] = {0};
    size_t length = strlen(text);
    if (length >= BLOCK_SIZE) {
        fprintf(stderr, "Text must be at most %d bytes\n", BLOCK_SIZE - 1);
        return 1;
    }
    memcpy(bytes, text, length);
    FILE *file = open_disk("r+b");
    if (!file) return 1;
    int ok = write_block(file, block, bytes);
    if (fclose(file) != 0) ok = 0;
    if (!ok) { fprintf(stderr, "Could not write block %zu\n", block); return 1; }
    printf("Wrote block %zu (offset %zu)\n", block, block * (size_t)BLOCK_SIZE);
    return 0;
}

int main(int argc, char **argv) {
    size_t block;
    if (argc == 2 && strcmp(argv[1], "init") == 0) return init_disk();
    if (argc == 2 && strcmp(argv[1], "info") == 0) return info_disk();
    if (argc >= 3 && parse_block(argv[2], &block)) {
        if (argc == 3 && strcmp(argv[1], "read") == 0) return display_block(block);
        if (argc == 4 && strcmp(argv[1], "write") == 0) return update_block(block, argv[3]);
        if (argc == 3 && strcmp(argv[1], "zero") == 0) return update_block(block, "");
    }
    fprintf(stderr, "Usage:\n  %s init\n  %s info\n  %s read BLOCK\n  %s write BLOCK TEXT\n  %s zero BLOCK\nValid block numbers: 0 to %d\n", argv[0], argv[0], argv[0], argv[0], argv[0], BLOCK_COUNT - 1);
    return 1;
}
