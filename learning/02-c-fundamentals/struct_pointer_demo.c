/* Learning 02: Structures, pointers, dynamic memory for a file system model.
   Educational in-memory example; does not create a real file system. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_CAPACITY 32

typedef struct {
    unsigned int inode_number;
    char name[NAME_CAPACITY];
    size_t size_bytes;
} FileMetadata;

static void print_file(const FileMetadata *file) {
    if (file == NULL) {
        return;
    }
    printf("inode=%u name=%s size=%zu bytes\n",
           file->inode_number, file->name, file->size_bytes);
}

static int update_size(FileMetadata *file, size_t new_size) {
    if (file == NULL) {
        return 0;
    }
    file->size_bytes = new_size;
    return 1;
}

int main(void) {
    const size_t count = 2;
    FileMetadata *files = calloc(count, sizeof *files);
    if (files == NULL) {
        fputs("Could not allocate file metadata.\n", stderr);
        return EXIT_FAILURE;
    }

    files[0].inode_number = 1;
    snprintf(files[0].name, sizeof files[0].name, "%s", "report.txt");
    files[0].size_bytes = 1200;

    files[1].inode_number = 2;
    snprintf(files[1].name, sizeof files[1].name, "%s", "notes.txt");
    files[1].size_bytes = 64;

    puts("Before update:");
    for (size_t i = 0; i < count; ++i) {
        print_file(&files[i]);
    }

    if (!update_size(&files[0], 1536)) {
        free(files);
        return EXIT_FAILURE;
    }

    puts("After update:");
    print_file(&files[0]);

    free(files);
    files = NULL;
    return EXIT_SUCCESS;
}
