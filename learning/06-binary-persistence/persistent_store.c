#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define STORE_FILE "learning06_store.bin"
#define RECORD_COUNT 4u
#define NAME_SIZE 24u
#define RECORD_SIZE (4u + NAME_SIZE + 4u)
#define HEADER_SIZE 12u
#define VERSION 1u

static int write_u32(FILE *file, uint32_t value)
{
    unsigned char b[4];
    for (unsigned i = 0; i < 4; ++i)
        b[i] = (unsigned char)(value >> (i * 8u));
    return fwrite(b, 1, sizeof b, file) == sizeof b;
}

static int read_u32(FILE *file, uint32_t *value)
{
    unsigned char b[4];
    if (fread(b, 1, sizeof b, file) != sizeof b)
        return 0;
    *value = (uint32_t)b[0] | ((uint32_t)b[1] << 8) |
             ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
    return 1;
}

static int write_record(FILE *file, uint32_t id, const char *name, uint32_t size)
{
    unsigned char fixed_name[NAME_SIZE] = {0};
    size_t len = strlen(name);
    if (len >= NAME_SIZE)
        return 0;
    memcpy(fixed_name, name, len);
    return write_u32(file, id) &&
           fwrite(fixed_name, 1, sizeof fixed_name, file) == sizeof fixed_name &&
           write_u32(file, size);
}

static int open_validated(FILE **out)
{
    unsigned char magic[4];
    uint32_t version = 0, count = 0;
    FILE *file = fopen(STORE_FILE, "rb");
    if (file == NULL) {
        perror(STORE_FILE);
        return 0;
    }
    if (fread(magic, 1, 4, file) != 4 ||
        memcmp(magic, "L06S", 4) != 0 ||
        !read_u32(file, &version) || !read_u32(file, &count) ||
        version != VERSION || count != RECORD_COUNT) {
        fprintf(stderr, "Invalid store header or unsupported format\n");
        fclose(file);
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0 ||
        ftell(file) != (long)(HEADER_SIZE + RECORD_COUNT * RECORD_SIZE) ||
        fseek(file, HEADER_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "Store size is incorrect\n");
        fclose(file);
        return 0;
    }
    *out = file;
    return 1;
}

static int initialize_store(void)
{
    const char *names[RECORD_COUNT] = {"report.txt", "notes.txt", "draft.c", "data.bin"};
    uint32_t sizes[RECORD_COUNT] = {1200, 64, 512, 2048};
    FILE *file = fopen(STORE_FILE, "wb");
    if (file == NULL) {
        perror(STORE_FILE);
        return 0;
    }
    int ok = fwrite("L06S", 1, 4, file) == 4 &&
             write_u32(file, VERSION) && write_u32(file, RECORD_COUNT);
    for (uint32_t i = 0; i < RECORD_COUNT && ok; ++i)
        ok = write_record(file, i + 1, names[i], sizes[i]);
    if (fclose(file) != 0)
        ok = 0;
    if (!ok) {
        fprintf(stderr, "Failed to initialize store\n");
        return 0;
    }
    printf("Initialized %s with %u records\n", STORE_FILE, RECORD_COUNT);
    return 1;
}

static int show_store(void)
{
    FILE *file;
    if (!open_validated(&file))
        return 0;
    printf("ID  NAME                     SIZE\n");
    for (uint32_t i = 0; i < RECORD_COUNT; ++i) {
        uint32_t id, size;
        unsigned char name[NAME_SIZE];
        if (!read_u32(file, &id) ||
            fread(name, 1, NAME_SIZE, file) != NAME_SIZE ||
            !read_u32(file, &size)) {
            fprintf(stderr, "Truncated record\n");
            fclose(file);
            return 0;
        }
        name[NAME_SIZE - 1] = '\0';
        printf("%-3u %-24s %u\n", id, (char *)name, size);
    }
    return fclose(file) == 0;
}

static int update_size(uint32_t id, uint32_t size)
{
    FILE *check;
    if (id < 1 || id > RECORD_COUNT || !open_validated(&check))
        return 0;
    if (fclose(check) != 0)
        return 0;
    FILE *file = fopen(STORE_FILE, "r+b");
    if (file == NULL) {
        perror(STORE_FILE);
        return 0;
    }
    long offset = (long)(HEADER_SIZE + (id - 1u) * RECORD_SIZE + 4u + NAME_SIZE);
    int ok = fseek(file, offset, SEEK_SET) == 0 && write_u32(file, size);
    if (fclose(file) != 0)
        ok = 0;
    if (ok)
        printf("Updated record %u to %u bytes\n", id, size);
    else
        fprintf(stderr, "Update failed\n");
    return ok;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "init") == 0)
        return initialize_store() ? EXIT_SUCCESS : EXIT_FAILURE;
    if (argc == 2 && strcmp(argv[1], "show") == 0)
        return show_store() ? EXIT_SUCCESS : EXIT_FAILURE;
    if (argc == 4 && strcmp(argv[1], "update") == 0) {
        char *end1, *end2;
        unsigned long id = strtoul(argv[2], &end1, 10);
        unsigned long size = strtoul(argv[3], &end2, 10);
        if (*argv[2] == '\0' || *argv[3] == '\0' ||
            *end1 != '\0' || *end2 != '\0' ||
            id < 1 || id > RECORD_COUNT || size > UINT32_MAX) {
            fprintf(stderr, "Invalid id or size\n");
            return EXIT_FAILURE;
        }
        return update_size((uint32_t)id, (uint32_t)size)
                   ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    fprintf(stderr, "Usage: %s init | show | update <id> <size>\n", argv[0]);
    return EXIT_FAILURE;
}
