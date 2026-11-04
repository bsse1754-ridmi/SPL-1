# Learning 03 — Bitwise Operations in C

**SPL-1 | File System Simulator | Learning Series**

## Purpose

Learn how binary values and bitwise operations allow a file system to efficiently track free and allocated blocks. This module is a **standalone, in-memory demonstration**, not the integrated file system implementation.

## Learning objectives

After this module, you should be able to:

1. Convert numbers between binary, decimal, and hexadecimal.
2. Explain the difference between `&`, `|`, `^`, `~`, `<<`, `>>` and logical operators like `&&`.
3. Set, clear, and test individual bits with masks.
4. Explain how a bitmap represents free and allocated blocks.
5. Implement and test block allocation with input validation.

## University PDFs to study (original sources)

**Download or read the PDFs from their authors' websites.** They are linked here rather than redistributed in this repository.

1. **Stanford University — CS107 Lecture 3: Bits and Bytes; Bitwise Operators (PDF)**  
   https://web.stanford.edu/class/archive/cs/cs107/cs107.1206/lectures/3/Lecture3.pdf  
   Study: binary representation, AND/OR/XOR/NOT, bit masks and shifts.

2. **University at Buffalo — CSE 220: Bitwise Operations (PDF)**  
   https://cse.buffalo.edu/~eblanton/course/cse220-2021-0s/materials/16-bitwise.pdf  
   Study: truth tables, bitwise vs. logical operators, masks, set/clear/toggle.

**Additional online notes:** University of Chicago, Lecture 3 (bit manipulation): https://www.classes.cs.uchicago.edu/current/14300-1/lectures/lec03_notes/

## Concept: binary bitmap

The demonstration models 16 disk blocks with **two bytes** (`uint8_t bitmap[2]`). Each bit corresponds to one block:

- `0` means free
- `1` means allocated
- block 0 is the least-significant bit of byte 0
- block 8 is the least-significant bit of byte 1

```
Block number:    0 1 2 3 4 5 6 7 | 8 9 10 11 12 13 14 15
Initial bitmap:  0 0 0 0 0 0 0 0 | 0 0  0  0  0  0  0  0
After 0,3,9:    1 0 0 1 0 0 0 0 | 0 1  0  0  0  0  0  0
After free(3):  1 0 0 0 0 0 0 0 | 0 1  0  0  0  0  0  0
```

The program prints bits in increasing block-number order (not the conventional most-significant-bit-first binary notation).

### Essential expressions

For a valid block index `block`:

```c
size_t byte_index = block / 8;
size_t bit_index = block % 8;
uint8_t mask = (uint8_t)(1u << bit_index);

bitmap[byte_index] |= mask;                 // allocate (set bit)
bitmap[byte_index] &= (uint8_t)~mask;       // release (clear bit)
int used = (bitmap[byte_index] & mask) != 0;// test bit
```

Always validate the index before accessing the bitmap. Using unsigned operands avoids common issues with signed bit shifting.

## Files

| File | Description |
| --- | --- |
| `README.md` | Learning guide, original-source PDF links, build steps and exercises |
| `bitwise_bitmap_demo.c` | Runnable C11 demonstration of six operators and a 16-block bitmap |

## Build and run on macOS / Linux (from this folder)

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 bitwise_bitmap_demo.c -o bitwise_bitmap_demo
./bitwise_bitmap_demo
```

On Windows with MinGW/GCC:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 bitwise_bitmap_demo.c -o bitwise_bitmap_demo.exe
.\bitwise_bitmap_demo.exe
```

## Expected output

```text
Bitwise operators (a=12, b=10):
AND: 8
OR : 14
XOR: 6
NOT (low 8 bits of a): 243
a << 1: 24
a >> 1: 6

File-system block bitmap (1=allocated, 0=free):
Blocks  0..15 : 00000000 00000000
Allocate block 0: OK
Allocate block 3: OK
Allocate block 9: OK
Blocks  0..15 : 10010000 01000000
Allocate block 3 again: FAILED (already allocated)
Free block 3: OK
Blocks  0..15 : 10000000 01000000
First free block: 1
Block 9 allocated? YES
Invalid block 16 allocation: REJECTED
Bitmap storage: 2 bytes for 16 blocks
```

## Exercises

1. Explain why `12 & 10` is `8` but `12 && 10` is `1`.
2. Allocate blocks 7, 8 and 15; predict the printed bitmap before compiling.
3. Try freeing an already-free block. What does the function return?
4. Modify the demo for `BLOCK_COUNT = 24` and confirm the storage becomes three bytes.
5. Extend the program with a function to count allocated blocks.

## Connection to SPL-1

Later modules will use a persistent allocation bitmap as part of the virtual disk layout, along with superblock and inode metadata. This module only teaches the bit manipulation required for that feature; it does not modify the virtual disk.

**Next:** Learning 04 — Modular Programming in C.
