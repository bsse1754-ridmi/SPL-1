/*
 * File System Fundamentals - Learning 01
 * A portable C11 demonstration of basic file operations through stdio.
 * This is NOT a file system implementation. It uses the host OS file system.
 */
#include <stdio.h>
#include <string.h>

#define DEMO_FILE "fs_intro_sample.txt"
#define BUFFER_SIZE 256

static int show_contents(const char *path) {
    FILE *fp = fopen(path, "r");
    char buffer[BUFFER_SIZE];
    if (fp == NULL) {
        perror("Cannot open file for reading");
        return 1;
    }
    printf("Contents of %s:\n", path);
    while (fgets(buffer, sizeof buffer, fp) != NULL) {
        fputs(buffer, stdout);
    }
    if (ferror(fp)) {
        perror("Error reading file");
        fclose(fp);
        return 1;
    }
    if (fclose(fp) == EOF) {
        perror("Error closing file");
        return 1;
    }
    return 0;
}

static int add_text(const char *path, const char *mode, const char *text) {
    FILE *fp = fopen(path, mode);
    if (fp == NULL) {
        perror("Cannot open file for writing");
        return 1;
    }
    if (fputs(text, fp) == EOF) {
        perror("Error writing file");
        fclose(fp);
        return 1;
    }
    if (fclose(fp) == EOF) {
        perror("Error closing file");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--clean") == 0) {
        if (remove(DEMO_FILE) != 0) {
            perror("Could not remove demo file");
            return 1;
        }
        printf("Removed %s\n", DEMO_FILE);
        return 0;
    }
    if (argc != 1) {
        fprintf(stderr, "Usage: %s [--clean]\n", argv[0]);
        return 1;
    }

    /* C11 x mode makes creation fail if the target file exists. */
    if (add_text(DEMO_FILE, "wx", "Hello from a C file!\n") != 0) {
        fprintf(stderr, "Demo stopped. If the file already exists, run --clean first.\n");
        return 1;
    }
    puts("1. Created a file and wrote the first line.");
    if (show_contents(DEMO_FILE) != 0) return 1;

    if (add_text(DEMO_FILE, "a", "This line was appended.\n") != 0) return 1;
    puts("2. Appended another line.");
    if (show_contents(DEMO_FILE) != 0) return 1;

    printf("3. The file remains on disk as %s.\n", DEMO_FILE);
    puts("4. Run the program with --clean to remove the demo file.");
    return 0;
}
