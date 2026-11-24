#ifndef LEARNING10_SUPERBLOCK_H
#define LEARNING10_SUPERBLOCK_H
#include <stddef.h>
#include <stdint.h>
#define SB_IMAGE "learning10_disk.bin"
#define SB_BLOCK_SIZE 512u
#define SB_TOTAL_BLOCKS 128u
#define SB_DATA_START 11u
#define SB_MAGIC 0x53464C31u
#define SB_VERSION 1u
#define SB_HEADER_BYTES 64u
int sb_create(const char *path);
int sb_verify(const char *path);
int sb_inspect(const char *path);
int sb_corrupt(const char *path);
#endif
