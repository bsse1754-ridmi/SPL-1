# Learning 04 — Modular Programming in C

**SPL-1 | File System Simulator | Learning Series**

## Purpose

Learn how to organize a C program into separate `.c` implementation files and `.h` interface files, compile independently, link object files, and maintain a small Makefile. This program is a **standalone educational demonstration**. It is not the complete File System Simulator and does not persist data to a virtual disk.

## Learning objectives

1. Understand translation units, preprocessing, compilation, linking, and object files.
2. Explain declarations versus definitions and use header include guards.
3. Separate public APIs (`bitmap.h`, `report.h`) from implementations (`bitmap.c`, `report.c`).
4. Understand `static` internal-linkage helper functions and `const` parameters.
5. Use a `Makefile` to compile only changed modules.
6. Connect modular code organization to the future SPL-1 file system architecture.

## External PDF learning resources (links, not redistributed copies)

**Read in this order:**

1. **University of Washington, CSE 374 — Multi-File Programs, Header Files & Preprocessor** (core lesson): https://courses.cs.washington.edu/courses/cse374/23wi/lectures/13-cpp.pdf
2. **University of Washington, CSE 374 — Header Files / Separate Compilation Review** (rules and revision): https://courses.cs.washington.edu/courses/cse374/22wi/lectures/Lecture26.pdf
3. **Carnegie Mellon University — Transition to C** (separate compilation and linking): https://www.cs.cmu.edu/~15122-archive/n25/handouts/slides/review/17-cintro.pdf
4. **University of Washington, CSE 333 — Data Structures and Modules** (more advanced study): https://courses.cs.washington.edu/courses/cse333/24sp/lectures/05-c-modules.pdf
5. **MIT OpenCourseWare — Compilation Pipeline** (lecture page with a downloadable PDF and C lab): https://ocw.mit.edu/courses/6-s096-introduction-to-c-and-c-january-iap-2013/pages/lectures-and-assignments/compilation-pipeline/

The original authors retain their rights. Link to the PDFs from GitHub rather than uploading copies without permission.

## Directory layout

```text
04-modular-programming/
├── README.md
├── main.c
├── bitmap.h
├── bitmap.c
├── report.h
├── report.c
└── Makefile
```

## Module roles

- `bitmap.h`: public data structure, constants, and operation declarations.
- `bitmap.c`: private validation and bitmap state operations. Returns `1` for changed, `0` for unchanged, `-1` for invalid input.
- `report.h`: declaration for the reporting API.
- `report.c`: formatted text output, without changing the bitmap.
- `main.c`: a tiny application that calls the APIs.
- `Makefile`: compile/link rules and header dependencies.

## Build and run

From this folder on macOS or Linux:

```bash
make
./modular_demo
make clean
```

Alternative without Make:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror main.c bitmap.c report.c -o modular_demo
./modular_demo
```

## Expected output

```text
Learning 04: Modular C file system demo
Allocate block 0: 1
Allocate block 9: 1
Allocate block 9 again: 0
Release block 0: 1
Allocate invalid block 32: -1
Blocks: 32 | Allocated: 1 | Free: 31
Bitmap: 00000000 01000000 00000000 00000000
```

## Concepts to explain in your GitHub submission

- Why include guards prevent multiple inclusion problems within a translation unit.
- Why a function prototype belongs in a header while its normal definition belongs in one `.c` file.
- Why `static valid_block()` is private to `bitmap.c`.
- Why `#include "bitmap.h"` is different from including a C implementation file directly.
- Why `main.o`, `bitmap.o`, and `report.o` are generated build artifacts, not source code to commit.
- Why editing `bitmap.h` triggers recompilation of dependent objects when using this Makefile.

## Exercises

1. Add `bitmap_find_first_free()` to `bitmap.h` and `bitmap.c` and use it from `main.c`.
2. Add `bitmap_allocate_first_free()` that returns the chosen block index or `-1` on failure.
3. Add a unit-test-style `test_bitmap.c` and a `make test` target.
4. Deliberately remove the definition of `bitmap_release()` and observe the linker error; then restore it.
5. Change a public API declaration in the header and fix all modules to match it.

## Relation to the SPL-1 project

The same module structure will later be used for `disk.c`/`disk.h`, `block.c`/`block.h`, `inode.c`/`inode.h`, and directory/path modules. This demonstration keeps its APIs self-contained and does not modify the actual integrated project.

**Next:** Learning 05 — File Handling in C.
