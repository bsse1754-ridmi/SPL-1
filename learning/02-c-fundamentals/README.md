# Learning 02 — C Fundamentals for File System Development

**Project:** SPL-1 — File System Simulator in C  
**Module:** 02 of 30  
**Focus:** Structures, pointers, arrays, functions and dynamic memory.

## Learning objectives

By the end of this module, you should be able to:

1. Define a `struct` to group file metadata (inode number, name and size).
2. Explain `&` (address-of), `*` (dereference) and `->` (access a member through a struct pointer).
3. Pass structures to functions using `const` pointers and writable pointers.
4. Allocate an array of structures using `calloc`, check allocation success and release it with `free`.
5. Describe stack versus dynamically allocated storage and avoid common mistakes such as using freed memory.

## External PDF learning resources

These PDFs are **published by their respective authors/institutions**, not authored by this repository. Follow the links to read them; do not add copies to the repository unless their license permits redistribution.

1. **Beej's Guide to C Programming (PDF):** https://beej.us/guide/bgc/pdf/bgc_a4_c_1.pdf  
   Read the sections on pointers, arrays, structs and manual memory allocation. This is a comprehensive reference.
2. **University of Minnesota — Pointers & Memory Management in C (PDF):** https://www-users.cse.umn.edu/~shekhar/4061/notes/memorymgmt.pdf  
   Focus on pointer diagrams, `malloc`/`calloc`/`free` and memory leaks.
3. **MIT OpenCourseWare — Lecture 2: The Logistics of Pointers and Memory Management:** https://ocw.mit.edu/courses/6-088-introduction-to-c-memory-management-and-c-object-oriented-programming-january-iap-2010/resources/mit6_088iap10_lec02/  
   Open the page and select **Download File** for its PDF (pointers, stack and heap, arrays and structs).

## Repository contents

- `README.md` — learning guide, source links, exercises and instructions.
- `struct_pointer_demo.c` — standalone C11 program that models file metadata in memory.

## Compile and run

From this directory:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 struct_pointer_demo.c -o struct_pointer_demo
./struct_pointer_demo
```

On Windows PowerShell, compile with `-o struct_pointer_demo.exe` and run `./struct_pointer_demo.exe`.

Expected output:

```text
Before update:
inode=1 name=report.txt size=1200 bytes
inode=2 name=notes.txt size=64 bytes
After update:
inode=1 name=report.txt size=1536 bytes
```

## How the C example maps to a file system

| C concept | Example | Later use in SPL-1 |
| --- | --- | --- |
| `struct` | `FileMetadata` | Inode and directory-entry models |
| Array | `files[0]`, `files[1]` | Inode tables and block metadata |
| Pointer | `FileMetadata *files` | Accessing dynamically allocated records |
| `const` pointer parameter | `print_file` | Reading metadata without changing it |
| Writable pointer parameter | `update_size` | Updating a record's fields |
| `calloc` / `free` | Allocate/release records | Managing temporary runtime memory |

**Important:** This demo stores metadata only in RAM and uses the host operating system's memory allocator. It does not implement a disk image, persistent storage, or your own file-system allocator. Those topics come later.

## Study exercises

1. Add a `type` field to distinguish a regular file from a directory.
2. Change the metadata array to store three files and print them all.
3. Write a `find_file_by_inode` function returning a pointer to a matching entry, or `NULL` if none exists.
4. Explain what happens if you access `files[0]` after `free(files)` (do **not** deliberately depend on this undefined behavior).
5. Explain why `snprintf` is safer than copying an arbitrarily long string into a fixed-length `name` field.

## Self-check

- Can you explain `file.name` versus `file_ptr->name`?
- Can you explain what `sizeof *files` does in `calloc`?
- Can you identify who owns allocated memory and where it is freed?

## Suggested commit

```bash
git add learning/02-c-fundamentals/
git commit -m "learn: add C structures pointers and memory management"
git push origin main
```

## Next module

**Learning 03 — Bitwise Operations in C**, preparing for block and inode allocation bitmaps.
