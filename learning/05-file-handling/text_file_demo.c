#include <stdio.h>
#include <string.h>

#define FILE_NAME "learning05_notes.txt"
#define BUFFER_SIZE 256

int main(void)
{
    const char *lines[] = {
        "File System Simulator - Learning 05\n",
        "Text mode stores readable characters.\n",
        "Always check file operations.\n"
    };
    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        perror("Cannot create text file");
        return 1;
    }
    for (size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
        if (fputs(lines[i], file) == EOF) {
            perror("Writing text failed");
            fclose(file);
            return 1;
        }
    }
    if (fclose(file) == EOF) {
        perror("Closing text file failed");
        return 1;
    }

    file = fopen(FILE_NAME, "a");
    if (file == NULL) {
        perror("Cannot append to text file");
        return 1;
    }
    if (fputs("Append mode keeps existing contents.\n", file) == EOF) {
        perror("Appending text failed");
        fclose(file);
        return 1;
    }
    if (fclose(file) == EOF) {
        perror("Closing appended file failed");
        return 1;
    }

    file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        perror("Cannot read text file");
        return 1;
    }
    printf("=== Text file contents ===\n");
    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file) != NULL)
        fputs(line, stdout);
    if (ferror(file)) {
        perror("Reading text failed");
        fclose(file);
        return 1;
    }
    if (fclose(file) == EOF) {
        perror("Closing read stream failed");
        return 1;
    }
    printf("Created: %s\n", FILE_NAME);
    return 0;
}
