#include "fs.h"
static int raw_validate_index(int index)
{
    if (!super_is_ready())
        return FS_NOT_MOUNTED;
    if (index < FS_DATA_START || index >= FS_BLOCK_COUNT)
        return FS_INVALID;
    if (fs.bitmap[index] != 1)
        return FS_NOT_FOUND;
    return FS_OK;
}
static int raw_validate_count(int count)
{
    if (count < 0 || count > FS_BLOCK_SIZE)
        return FS_INVALID;
    return FS_OK;
}
static int raw_is_printable(unsigned char ch)
{
    if (ch >= 32 && ch <= 126)
        return 1;
    return 0;
}
static int raw_get_buffer(int index, unsigned char *data)
{
    int result;
    if (data == NULL)
        return FS_INVALID;
    result = raw_validate_index(index);
    if (result != FS_OK)
        return result;
    return block_read_data(index, data);
}
static int raw_put_buffer(int index, const unsigned char *data)
{
    int result;
    if (data == NULL)
        return FS_INVALID;
    result = raw_validate_index(index);
    if (result != FS_OK)
        return result;
    result = block_write_data(index, data);
    if (result != FS_OK)
        return result;
    return disk_flush();
}
int raw_write_text(int index, const char *text)
{
    unsigned char data[FS_BLOCK_SIZE];
    size_t length;
    if (text == NULL)
        return FS_INVALID;
    length = strlen(text);
    if (length > FS_BLOCK_SIZE)
        return FS_TOO_LARGE;
    memset(data, 0, sizeof(data));
    memcpy(data, text, length);
    return raw_put_buffer(index, data);
}
int raw_read_text(int index)
{
    unsigned char data[FS_BLOCK_SIZE];
    int result;
    int length = 0;
    result = raw_get_buffer(index, data);
    if (result != FS_OK)
        return result;
    while (length < FS_BLOCK_SIZE && data[length] != 0)
        length++;
    if (length > 0)
        fwrite(data, 1, (size_t)length, stdout);
    putchar('\n');
    return FS_OK;
}
int raw_fill(int index, unsigned char value)
{
    unsigned char data[FS_BLOCK_SIZE];
    int i;
    for (i = 0; i < FS_BLOCK_SIZE; i++)
        data[i] = value;
    return raw_put_buffer(index, data);
}
int raw_zero(int index)
{
    return raw_fill(index, 0);
}
int raw_copy(int source, int destination)
{
    unsigned char data[FS_BLOCK_SIZE];
    int result;
    if (source == destination)
        return raw_validate_index(source);
    result = raw_get_buffer(source, data);
    if (result != FS_OK)
        return result;
    return raw_put_buffer(destination, data);
}
int raw_compare(int left, int right)
{
    unsigned char first[FS_BLOCK_SIZE];
    unsigned char second[FS_BLOCK_SIZE];
    int result;
    int i;
    result = raw_get_buffer(left, first);
    if (result != FS_OK)
        return result;
    result = raw_get_buffer(right, second);
    if (result != FS_OK)
        return result;
    for (i = 0; i < FS_BLOCK_SIZE; i++)
    {
        if (first[i] != second[i])
            return 0;
    }
    return 1;
}
static void raw_print_hex_byte(unsigned char value)
{
    const char *digits = "0123456789ABCDEF";
    putchar(digits[value >> 4]);
    putchar(digits[value & 15]);
}
static void raw_print_hex_row(const unsigned char *data, int begin, int count)
{
    int j;
    printf("%04X  ", begin);
    for (j = 0; j < 16; j++)
    {
        if (begin + j < count)
            raw_print_hex_byte(data[begin + j]);
        else
            printf("  ");
        putchar(' ');
    }
    printf(" | ");
    for (j = 0; j < 16 && begin + j < count; j++)
    {
        unsigned char ch = data[begin + j];
        putchar(raw_is_printable(ch) ? ch : '.');
    }
    putchar('\n');
}
int raw_show_hex(int index, int count)
{
    unsigned char data[FS_BLOCK_SIZE];
    int result;
    int i;
    result = raw_validate_count(count);
    if (result != FS_OK)
        return result;
    result = raw_get_buffer(index, data);
    if (result != FS_OK)
        return result;
    printf("Block %d, first %d bytes:\n", index, count);
    for (i = 0; i < count; i += 16)
        raw_print_hex_row(data, i, count);
    return FS_OK;
}
int raw_show_text(int index, int count)
{
    unsigned char data[FS_BLOCK_SIZE];
    int result;
    int i;
    result = raw_validate_count(count);
    if (result != FS_OK)
        return result;
    result = raw_get_buffer(index, data);
    if (result != FS_OK)
        return result;
    for (i = 0; i < count; i++)
    {
        if (raw_is_printable(data[i]))
            putchar(data[i]);
        else if (data[i] == '\n' || data[i] == '\t')
            putchar(data[i]);
        else
            putchar('.');
    }
    putchar('\n');
    return FS_OK;
}
int raw_count_value(int index, unsigned char value)
{
    unsigned char data[FS_BLOCK_SIZE];
    int result;
    int total = 0;
    int i;
    result = raw_get_buffer(index, data);
    if (result != FS_OK)
        return result;
    for (i = 0; i < FS_BLOCK_SIZE; i++)
    {
        if (data[i] == value)
            total++;
    }
    return total;
}
int raw_checksum(int index, unsigned long *output)
{
    unsigned char data[FS_BLOCK_SIZE];
    unsigned long hash = 2166136261UL;
    int result;
    int i;
    if (output == NULL)
        return FS_INVALID;
    result = raw_get_buffer(index, data);
    if (result != FS_OK)
        return result;
    for (i = 0; i < FS_BLOCK_SIZE; i++)
    {
        hash ^= (unsigned long)data[i];
        hash = (hash * 16777619UL) & 0xFFFFFFFFUL;
    }
    *output = hash;
    return FS_OK;
}
static int raw_count_used_bytes(const unsigned char *data)
{
    int i;
    int total = 0;
    for (i = 0; i < FS_BLOCK_SIZE; i++)
    {
        if (data[i] != 0)
            total++;
    }
    return total;
}
static int raw_count_printable_bytes(const unsigned char *data)
{
    int i;
    int total = 0;
    for (i = 0; i < FS_BLOCK_SIZE; i++)
    {
        if (raw_is_printable(data[i]))
            total++;
    }
    return total;
}
static int raw_count_changes(const unsigned char *data)
{
    int i;
    int changes = 0;
    for (i = 1; i < FS_BLOCK_SIZE; i++)
    {
        if (data[i] != data[i - 1])
            changes++;
    }
    return changes;
}
int raw_inspect(int index)
{
    unsigned char data[FS_BLOCK_SIZE];
    unsigned long hash;
    int result;
    result = raw_get_buffer(index, data);
    if (result != FS_OK)
        return result;
    result = raw_checksum(index, &hash);
    if (result != FS_OK)
        return result;
    printf("--- Block %d inspection ---\n", index);
    printf("Offset in disk: %ld\n", (long)index * FS_BLOCK_SIZE);
    printf("Allocated: yes\n");
    printf("Capacity: %d bytes\n", FS_BLOCK_SIZE);
    printf("Nonzero bytes: %d\n", raw_count_used_bytes(data));
    printf("Printable bytes: %d\n", raw_count_printable_bytes(data));
    printf("Adjacent byte changes: %d\n", raw_count_changes(data));
    printf("Checksum (FNV-1a): %08lX\n", hash);
    return FS_OK;
}
int raw_write_pattern(int index, const char *pattern)
{
    unsigned char data[FS_BLOCK_SIZE];
    size_t length;
    int i;
    if (pattern == NULL)
        return FS_INVALID;
    length = strlen(pattern);
    if (length == 0 || length > FS_BLOCK_SIZE)
        return FS_INVALID;
    for (i = 0; i < FS_BLOCK_SIZE; i++)
        data[i] = (unsigned char)pattern[i % length];
    return raw_put_buffer(index, data);
}
int raw_find_text(int index, const char *needle)
{
    unsigned char data[FS_BLOCK_SIZE];
    size_t length;
    int result;
    int start;
    int j;
    if (needle == NULL)
        return FS_INVALID;
    length = strlen(needle);
    if (length == 0 || length > FS_BLOCK_SIZE)
        return FS_INVALID;
    result = raw_get_buffer(index, data);
    if (result != FS_OK)
        return result;
    for (start = 0; start <= FS_BLOCK_SIZE - (int)length; start++)
    {
        for (j = 0; j < (int)length; j++)
        {
            if (data[start + j] != (unsigned char)needle[j])
                break;
        }
        if (j == (int)length)
            return start;
    }
    return FS_NOT_FOUND;
}
