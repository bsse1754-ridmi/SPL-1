#include "fs.h"
#define SHELL_LINE_LIMIT 4096
#define SHELL_COMMAND_LIMIT 64
typedef struct {
    int interactive;
    int banner;
    int run_check;
    int exit_requested;
    int commands;
    int succeeded;
    int failed;
    int line_number;
    char batch_filename[FS_PATH_LIMIT];
} ShellState;
static void shell_init(ShellState *state)
{
    memset(state, 0, sizeof(*state));
    state->interactive = 1;
    state->banner = 1;
}
static void shell_help(void)
{
    printf("--- STORAGE-LAYER VIRTUAL FILE SYSTEM ---\n");
    printf("format                   Initialize or erase virtual disk\n");
    printf("info                     Display disk and filesystem layout\n");
    printf("status                   Display mount and integrity status\n");
    printf("bitmap                   Display allocation bitmap preview\n");
    printf("map                      Display reserved/allocated block ranges\n");
    printf("stats                    Display block allocation statistics\n");
    printf("alloc                    Allocate the first free data block\n");
    printf("allocat <block>          Allocate one specified data block\n");
    printf("free <block>             Release an allocated data block\n");
    printf("used <block>             Report whether a block is allocated\n");
    printf("writeblock <block> <text> Store text directly in a data block\n");
    printf("readblock <block>        Display text stored in a data block\n");
    printf("hex <block> [bytes]      Display block bytes in hexadecimal\n");
    printf("text <block> [bytes]     Display printable block bytes\n");
    printf("fill <block> <byte>      Fill block with decimal byte 0-255\n");
    printf("pattern <block> <text>   Repeat a text pattern across block\n");
    printf("zero <block>             Clear contents without freeing block\n");
    printf("copy <source> <target>   Copy bytes between allocated blocks\n");
    printf("compare <left> <right>   Compare allocated block contents\n");
    printf("find <block> <text>      Find byte offset of text in a block\n");
    printf("inspect <block>          Show data block statistics/checksum\n");
    printf("checksum <block>         Compute data block checksum\n");
    printf("countbyte <block> <byte> Count occurrences of one byte\n");
    printf("check                    Validate storage metadata\n");
    printf("diagnostics              Display raw disk I/O diagnostics\n");
    printf("help                     Display this command reference\n");
    printf("exit                     Close storage and leave program\n");
    printf("This phase uses raw blocks; there are no named files or paths.\n");
}
static void shell_usage(const char *program)
{
    printf("Usage: %s [--help] [--version] [--no-banner] [--check] [--batch commands.txt]\n", program);
}
static int shell_set_batch(ShellState *state, const char *filename)
{
    if (filename == NULL || filename[0] == '\0')
        return FS_INVALID;
    if (strlen(filename) >= sizeof(state->batch_filename))
        return FS_TOO_LARGE;
    strcpy(state->batch_filename, filename);
    state->interactive = 0;
    state->banner = 0;
    return FS_OK;
}
static int shell_options(ShellState *state, int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            shell_usage(argv[0]);
            return 1;
        }
        if (strcmp(argv[i], "--version") == 0)
        {
            printf("Virtual File System storage-layer midterm, format version %d\n", FS_VERSION);
            return 1;
        }
        if (strcmp(argv[i], "--no-banner") == 0)
        {
            state->banner = 0;
            continue;
        }
        if (strcmp(argv[i], "--check") == 0)
        {
            state->run_check = 1;
            continue;
        }
        if (strcmp(argv[i], "--batch") == 0)
        {
            if (++i >= argc)
                return FS_INVALID;
            if (shell_set_batch(state, argv[i]) != FS_OK)
                return FS_INVALID;
            continue;
        }
        printf("Unknown program argument: %s\n", argv[i]);
        return FS_INVALID;
    }
    if (state->run_check && !state->interactive)
        return FS_INVALID;
    return FS_OK;
}
static char *shell_skip_spaces(char *text)
{
    while (*text && isspace((unsigned char)*text))
        text++;
    return text;
}
static void shell_trim_right(char *text)
{
    size_t length = strlen(text);
    while (length && isspace((unsigned char)text[length - 1]))
        text[--length] = '\0';
}
static int shell_read_line(FILE *input, char *line)
{
    size_t length;
    int ch;
    if (fgets(line, SHELL_LINE_LIMIT, input) == NULL)
        return ferror(input) ? FS_IO_ERROR : 1;
    length = strlen(line);
    if (length > 0 && line[length - 1] == '\n')
    {
        line[--length] = '\0';
    }
    else if (length == SHELL_LINE_LIMIT - 1 && !feof(input))
    {
        do {
            ch = fgetc(input);
        } while (ch != '\n' && ch != EOF);
        return FS_TOO_LARGE;
    }
    shell_trim_right(line);
    return FS_OK;
}
static int shell_parse(char *line, char *command, char **args)
{
    char *start = shell_skip_spaces(line);
    char *p = start;
    size_t length;
    if (*start == '\0' || *start == '#')
    {
        command[0] = '\0';
        *args = start;
        return FS_OK;
    }
    while (*p && !isspace((unsigned char)*p))
        p++;
    length = (size_t)(p - start);
    if (length >= SHELL_COMMAND_LIMIT)
        return FS_TOO_LARGE;
    memcpy(command, start, length);
    command[length] = '\0';
    for (length = 0; command[length]; length++)
        command[length] = (char)tolower((unsigned char)command[length]);
    if (*p)
        *p++ = '\0';
    *args = shell_skip_spaces(p);
    return FS_OK;
}
static int shell_parse_int(const char *text, int *output)
{
    long value = 0;
    int i = 0;
    if (text == NULL || *text == '\0' || output == NULL)
        return FS_INVALID;
    if (text[0] == '+')
        i++;
    if (!isdigit((unsigned char)text[i]))
        return FS_INVALID;
    while (text[i])
    {
        if (!isdigit((unsigned char)text[i]))
            return FS_INVALID;
        value = value * 10 + (text[i] - '0');
        if (value > 1000000)
            return FS_INVALID;
        i++;
    }
    *output = (int)value;
    return FS_OK;
}
static int shell_one_int(char *args, int *number)
{
    char *p = args;
    if (*p == '\0')
        return FS_INVALID;
    while (*p && !isspace((unsigned char)*p))
        p++;
    if (*p != '\0')
        return FS_INVALID;
    return shell_parse_int(args, number);
}
static int shell_two_ints(char *args, int *first, int *second)
{
    char *p = args;
    if (*p == '\0')
        return FS_INVALID;
    while (*p && !isspace((unsigned char)*p))
        p++;
    if (*p == '\0')
        return FS_INVALID;
    *p++ = '\0';
    p = shell_skip_spaces(p);
    if (shell_parse_int(args, first) != FS_OK)
        return FS_INVALID;
    return shell_one_int(p, second);
}
static int shell_block_and_text(char *args, int *block, char **text)
{
    char *p = args;
    if (*p == '\0')
        return FS_INVALID;
    while (*p && !isspace((unsigned char)*p))
        p++;
    if (*p == '\0')
        return FS_INVALID;
    *p++ = '\0';
    p = shell_skip_spaces(p);
    if (*p == '\0')
        return FS_INVALID;
    if (shell_parse_int(args, block) != FS_OK)
        return FS_INVALID;
    *text = p;
    return FS_OK;
}
static int shell_no_args(const char *args)
{
    return *args == '\0' ? FS_OK : FS_INVALID;
}
static int shell_need_mounted(void)
{
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    return FS_OK;
}
static int shell_run_alloc(char *args)
{
    int index;
    if (shell_no_args(args) != FS_OK)
        return FS_INVALID;
    index = block_allocate();
    if (index < 0)
        return index;
    printf("Allocated data block %d\n", index);
    return FS_OK;
}
static int shell_run_allocat(char *args)
{
    int index;
    int result;
    if (shell_one_int(args, &index) != FS_OK)
        return FS_INVALID;
    result = block_allocate_at(index);
    if (result == FS_OK)
        printf("Allocated data block %d\n", index);
    return result;
}
static int shell_run_free(char *args)
{
    int index;
    int result;
    if (shell_one_int(args, &index) != FS_OK)
        return FS_INVALID;
    result = block_release(index);
    if (result == FS_OK)
        printf("Released data block %d\n", index);
    return result;
}
static int shell_run_used(char *args)
{
    int index;
    int result;
    if (shell_one_int(args, &index) != FS_OK)
        return FS_INVALID;
    if (index < FS_DATA_START || index >= FS_BLOCK_COUNT)
        return FS_INVALID;
    result = block_is_allocated(index);
    if (result < 0)
        return result;
    printf("Block %d: %s\n", index, result == 1 ? "allocated" : "free");
    return FS_OK;
}
static int shell_run_block_text(char *args, int operation)
{
    int index;
    int result;
    char *text;
    result = shell_block_and_text(args, &index, &text);
    if (result != FS_OK)
        return result;
    if (operation == 1)
        result = raw_write_text(index, text);
    else if (operation == 2)
        result = raw_write_pattern(index, text);
    else
    {
        result = raw_find_text(index, text);
        if (result >= 0)
        {
            printf("Found at byte offset %d\n", result);
            return FS_OK;
        }
        return result;
    }
    if (result == FS_OK)
        printf("Wrote data to block %d\n", index);
    return result;
}
static int shell_run_hex_text(char *args, int hex)
{
    int index;
    int count = 64;
    int result;
    if (shell_one_int(args, &index) != FS_OK)
    {
        if (shell_two_ints(args, &index, &count) != FS_OK)
            return FS_INVALID;
    }
    if (hex)
        result = raw_show_hex(index, count);
    else
        result = raw_show_text(index, count);
    return result;
}
static int shell_run_compare(char *args)
{
    int left;
    int right;
    int result;
    if (shell_two_ints(args, &left, &right) != FS_OK)
        return FS_INVALID;
    result = raw_compare(left, right);
    if (result < 0)
        return result;
    printf("Blocks %d and %d: %s\n", left, right, result ? "identical" : "different");
    return FS_OK;
}
static int shell_run_copy(char *args)
{
    int source;
    int destination;
    int result;
    if (shell_two_ints(args, &source, &destination) != FS_OK)
        return FS_INVALID;
    result = raw_copy(source, destination);
    if (result == FS_OK)
        printf("Copied block %d to block %d\n", source, destination);
    return result;
}
static int shell_run_fill(char *args)
{
    int index;
    int value;
    if (shell_two_ints(args, &index, &value) != FS_OK)
        return FS_INVALID;
    if (value > 255)
        return FS_INVALID;
    return raw_fill(index, (unsigned char)value);
}
static int shell_run_countbyte(char *args)
{
    int index;
    int value;
    int result;
    if (shell_two_ints(args, &index, &value) != FS_OK)
        return FS_INVALID;
    if (value > 255)
        return FS_INVALID;
    result = raw_count_value(index, (unsigned char)value);
    if (result < 0)
        return result;
    printf("Byte value %d occurs %d times in block %d\n", value, result, index);
    return FS_OK;
}
static int shell_run_checksum(char *args)
{
    int index;
    int result;
    unsigned long hash;
    if (shell_one_int(args, &index) != FS_OK)
        return FS_INVALID;
    result = raw_checksum(index, &hash);
    if (result == FS_OK)
        printf("Block %d checksum: %08lX\n", index, hash);
    return result;
}
static int shell_run_one_block(char *args, int operation)
{
    int index;
    if (shell_one_int(args, &index) != FS_OK)
        return FS_INVALID;
    if (operation == 1)
        return raw_read_text(index);
    if (operation == 2)
        return raw_zero(index);
    return raw_inspect(index);
}
static int shell_dispatch(char *command, char *args)
{
    int result;
    if (command[0] == '\0')
        return FS_OK;
    if (strcmp(command, "help") == 0)
    {
        if (shell_no_args(args) != FS_OK)
            return FS_INVALID;
        shell_help();
        return FS_OK;
    }
    if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0)
        return shell_no_args(args) == FS_OK ? FS_BUSY : FS_INVALID;
    if (strcmp(command, "format") == 0)
    {
        if (shell_no_args(args) != FS_OK)
            return FS_INVALID;
        result = super_format();
        if (result == FS_OK)
            printf("Virtual disk formatted. Old data erased.\n");
        return result;
    }
    if (shell_need_mounted() != FS_OK)
        return FS_NOT_MOUNTED;
    if (strcmp(command, "info") == 0 && shell_no_args(args) == FS_OK)
    {
        super_print_info();
        return FS_OK;
    }
    if (strcmp(command, "status") == 0 && shell_no_args(args) == FS_OK)
    {
        super_print_status();
        return FS_OK;
    }
    if (strcmp(command, "bitmap") == 0 && shell_no_args(args) == FS_OK)
    {
        block_print_bitmap();
        return FS_OK;
    }
    if (strcmp(command, "map") == 0 && shell_no_args(args) == FS_OK)
    {
        block_print_map();
        return FS_OK;
    }
    if (strcmp(command, "stats") == 0 && shell_no_args(args) == FS_OK)
    {
        block_print_statistics();
        return FS_OK;
    }
    if (strcmp(command, "alloc") == 0)
        return shell_run_alloc(args);
    if (strcmp(command, "allocat") == 0)
        return shell_run_allocat(args);
    if (strcmp(command, "free") == 0)
        return shell_run_free(args);
    if (strcmp(command, "used") == 0)
        return shell_run_used(args);
    if (strcmp(command, "writeblock") == 0)
        return shell_run_block_text(args, 1);
    if (strcmp(command, "pattern") == 0)
        return shell_run_block_text(args, 2);
    if (strcmp(command, "find") == 0)
        return shell_run_block_text(args, 3);
    if (strcmp(command, "readblock") == 0)
        return shell_run_one_block(args, 1);
    if (strcmp(command, "zero") == 0)
        return shell_run_one_block(args, 2);
    if (strcmp(command, "inspect") == 0)
        return shell_run_one_block(args, 3);
    if (strcmp(command, "hex") == 0)
        return shell_run_hex_text(args, 1);
    if (strcmp(command, "text") == 0)
        return shell_run_hex_text(args, 0);
    if (strcmp(command, "copy") == 0)
        return shell_run_copy(args);
    if (strcmp(command, "compare") == 0)
        return shell_run_compare(args);
    if (strcmp(command, "fill") == 0)
        return shell_run_fill(args);
    if (strcmp(command, "countbyte") == 0)
        return shell_run_countbyte(args);
    if (strcmp(command, "checksum") == 0)
        return shell_run_checksum(args);
    if (strcmp(command, "check") == 0 && shell_no_args(args) == FS_OK)
    {
        result = super_check_consistency();
        printf("Metadata integrity check: %s\n", result == FS_OK ? "PASS" : "FAIL");
        return result;
    }
    if (strcmp(command, "diagnostics") == 0 && shell_no_args(args) == FS_OK)
    {
        disk_print_diagnostics();
        return FS_OK;
    }
    printf("Unknown command or invalid parameters: %s\n", command);
    return FS_INVALID;
}
static void shell_error(int result)
{
    if (result == FS_INVALID)
        printf("Error: invalid command parameters or block number.\n");
    else if (result == FS_NOT_MOUNTED)
        printf("Error: storage is not formatted; type format.\n");
    else if (result == FS_NO_SPACE)
        printf("Error: no free data blocks.\n");
    else if (result == FS_NOT_FOUND)
        printf("Error: block is free or requested text was not found.\n");
    else if (result == FS_EXISTS)
        printf("Error: block already allocated.\n");
    else if (result == FS_TOO_LARGE)
        printf("Error: data exceeds a block or input limit.\n");
    else if (result == FS_IO_ERROR)
        printf("Error: virtual disk I/O failure.\n");
    else
        printf("Error: operation failed (%d).\n", result);
}
static int shell_execute(ShellState *state, char *line)
{
    char command[SHELL_COMMAND_LIMIT];
    char *args;
    int result;
    result = shell_parse(line, command, &args);
    if (result != FS_OK)
    {
        state->failed++;
        shell_error(result);
        return result;
    }
    if (command[0] == '\0')
        return FS_OK;
    state->commands++;
    result = shell_dispatch(command, args);
    if (result == FS_BUSY)
    {
        state->exit_requested = 1;
        return FS_BUSY;
    }
    if (result == FS_OK)
        state->succeeded++;
    else
    {
        state->failed++;
        shell_error(result);
    }
    return result;
}
static void shell_summary(const ShellState *state)
{
    printf("Commands: %d, successful: %d, failed: %d\n", state->commands, state->succeeded, state->failed);
}
static int shell_interactive(ShellState *state)
{
    char line[SHELL_LINE_LIMIT];
    int result;
    while (!state->exit_requested)
    {
        printf(super_is_ready() ? "fs:blocks> " : "fs:unformatted> ");
        fflush(stdout);
        result = shell_read_line(stdin, line);
        if (result == 1)
            break;
        if (result != FS_OK)
        {
            state->failed++;
            shell_error(result);
            continue;
        }
        state->line_number++;
        result = shell_execute(state, line);
        if (result == FS_BUSY)
            break;
    }
    shell_summary(state);
    return FS_OK;
}
static int shell_batch(ShellState *state)
{
    FILE *input = fopen(state->batch_filename, "r");
    char line[SHELL_LINE_LIMIT];
    int result;
    int error = 0;
    if (input == NULL)
        return FS_IO_ERROR;
    while (!state->exit_requested)
    {
        result = shell_read_line(input, line);
        if (result == 1)
            break;
        state->line_number++;
        if (result != FS_OK)
        {
            state->failed++;
            error = 1;
            continue;
        }
        if (*shell_skip_spaces(line) == '\0' || *shell_skip_spaces(line) == '#')
            continue;
        printf("[%d] %s\n", state->line_number, line);
        result = shell_execute(state, line);
        if (result == FS_BUSY)
            break;
        if (result != FS_OK)
            error = 1;
    }
    if (fclose(input) != 0)
        error = 1;
    shell_summary(state);
    return error ? FS_ERROR : FS_OK;
}
int main(int argc, char **argv)
{
    ShellState state;
    int result;
    int close_result;
    shell_init(&state);
    result = shell_options(&state, argc, argv);
    if (result == 1)
        return EXIT_SUCCESS;
    if (result != FS_OK)
    {
        shell_usage(argv[0]);
        return EXIT_FAILURE;
    }
    result = disk_open();
    if (result != FS_OK)
    {
        shell_error(result);
        return EXIT_FAILURE;
    }
    result = super_mount();
    if (result != FS_OK)
        printf("No valid storage image mounted. Type format to initialize.\n");
    if (state.run_check)
    {
        result = super_is_ready() ? super_check_consistency() : FS_NOT_MOUNTED;
        printf("Integrity check: %s\n", result == FS_OK ? "PASS" : "FAIL");
    }
    else
    {
        if (state.banner)
        {
            printf("===========================================\n");
            printf(" VIRTUAL FILE SYSTEM - STORAGE MIDTERM\n");
            printf(" Disk + Superblock + Bitmap + Raw Blocks\n");
            printf("===========================================\n");
            printf("Type help to see the supported commands.\n");
        }
        result = state.interactive ? shell_interactive(&state) : shell_batch(&state);
    }
    close_result = disk_close();
    if (close_result != FS_OK)
        result = close_result;
    return result == FS_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
