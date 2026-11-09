<div align="center">

# 🗂️ SPL-1 | Virtual File System in C

### Systems Programming Lab I — Learning, Design & Implementation

**From C programming fundamentals to the architecture and development of a persistent, block-based virtual file system.**

![C](https://img.shields.io/badge/Language-C11-00599C?style=for-the-badge&logo=c&logoColor=white)
![GCC](https://img.shields.io/badge/Compiler-GCC-333333?style=for-the-badge)
![Project](https://img.shields.io/badge/Project-Virtual_File_System-197A6C?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-In_Development-7353BA?style=for-the-badge)

[📚 Learning Materials](https://github.com/bsse1754-ridmi/SPL-1/tree/main/learning) · [💻 Development Code](https://github.com/bsse1754-ridmi/SPL-1/tree/development/FileSystem) · [🌿 Development Branch](https://github.com/bsse1754-ridmi/SPL-1/tree/development)

</div>

---

## 📖 About This Repository

This repository documents my **Systems Programming Lab I (SPL-1)** project, combining a structured learning journey with the development of a **Virtual File System (VFS) in C**.

The learning materials cover programming and storage concepts needed to understand how a file system works. The practical project applies these concepts by simulating a disk using a binary file, managing blocks, tracking allocation, and interacting with storage through a command-line interface.

**Current milestone:** A storage-layer midterm prototype. The final goal is a more complete virtual file system with named files, inodes, directories, and path handling.

> **Scope note:** The current version reads and writes **raw data blocks**. It is not yet a complete file system with named files or folders. Inode, directory, and path-related features listed below are **planned**, not already implemented.

## 🧭 Quick Navigation

- [Learning Journey](#-learning-journey)
- [Project Objectives](#-project-objectives)
- [Current Implementation](#-current-implementation)
- [System Architecture](#-system-architecture)
- [Repository Structure](#-repository-structure)
- [Build and Run](#-build-and-run)
- [CLI Demonstration](#-cli-demonstration)
- [Development Roadmap](#-development-roadmap)
- [Git Branches](#-git-branches)

## 📚 Learning Journey

The [`learning/`](https://github.com/bsse1754-ridmi/SPL-1/tree/main/learning) folder tracks the topics studied as preparation for the project.

| # | Learning topic | Why it matters |
|:--:|:--|:--|
| 01 | File System Basics | Understand files, directories, metadata, and disk organization |
| 02 | C Fundamentals | Use functions, arrays, pointers, and structures in a systems program |
| 03 | Bitwise Operations | Understand flags, masks, and bitmap-based allocation |
| 04 | Modular Programming | Divide the code into cooperating C modules and headers |
| 05 | File Handling | Work with `FILE *`, binary I/O, and persistent data |
| 06 | Binary Persistence | Read and write structured data across program executions |
| 07 | Virtual Disk Architecture | Model fixed-sized storage blocks in a disk image |
| 08 | Blocks, Sectors & Offsets | Translate block numbers into byte offsets and inspect storage layout |

Learning resources and exercises are organized by topic so project development can be traced back to its underlying concepts.

## 🎯 Project Objectives

The project aims to:

1. Simulate persistent disk storage with a local binary file.
2. Organize storage into fixed-size blocks and implement block-level I/O.
3. Track free and occupied blocks using an allocation bitmap.
4. Store and validate essential disk metadata in a superblock.
5. Expose storage operations through a command-line interface.
6. Extend the storage foundation into a file system with inodes, files, directories, and path resolution in later phases.

## ✅ Current Implementation

The **midterm version** consists of six source files and focuses on block-level storage operations.

| Component | Current capabilities | Status |
|:--|:--|:--:|
| Virtual disk | Create/open binary disk image; read and write fixed-size blocks | ✅ |
| Superblock | Format, mount, inspect, and validate storage metadata | ✅ |
| Allocation bitmap | Allocate, release, and inspect data blocks | ✅ |
| Raw data operations | Write, read, copy, compare, fill, clear, and inspect block data | ✅ |
| Command-line interface | Interactive commands for disk and block operations | ✅ |
| Integrity checks | Verify storage-level metadata and allocation consistency | ✅ |
| Inode management | Manage named-file metadata and block ownership | ⏳ Planned |
| Files and directories | Create and manage named files and directory hierarchies | ⏳ Planned |
| Path resolution | Resolve absolute and relative paths | ⏳ Planned |

### Files in the Midterm Prototype

| File | Responsibility |
|:--|:--|
| `main.c` | Program entry point, command parsing, and interactive shell |
| `disk.c` | Disk-image access and block-level read/write functions |
| `superblock.c` | Formatting, mounting, metadata, and checks |
| `block.c` | Bitmap and free-block management |
| `file.c` | **Raw block content** operations (not named-file management yet) |
| `fs.h` | Shared constants, data structures, error codes, and function declarations |

## 🏗️ System Architecture

```text
                 +----------------------+
                 |     User / CLI       |
                 +-----------+----------+
                             |
                 +-----------v----------+
                 |   main.c (commands)  |
                 +-----------+----------+
                             |
           +-----------------+------------------+
           |                 |                  |
+----------v--------+ +------v---------+ +------v----------+
| file.c            | | block.c        | | superblock.c     |
| Raw data actions  | | Allocation     | | Disk metadata    |
+----------+--------+ +------+---------+ +------+----------+
           |                 |                  |
           +-----------------+------------------+
                             |
                 +-----------v----------+
                 |    disk.c (disk I/O) |
                 +-----------+----------+
                             |
                 +-----------v----------+
                 |   virtual_disk.bin   |
                 +----------------------+
```

`fs.h` provides the shared interfaces for these modules. The diagram illustrates their roles; the exact call order depends on the operation.

### 💾 Virtual Disk Layout

| Block range | Role | Space |
|:--|:--|--:|
| `0` | Superblock | 512 bytes |
| `1–4` | Allocation bitmap | 2 KiB |
| `5–36` | Reserved for future inode metadata | 16 KiB |
| `37–2047` | Allocatable data blocks | 2,011 blocks |

**Total disk capacity:** `2048 × 512 = 1,048,576 bytes (1 MiB)`.

The reserved inode region is **not used for inode management** in the current prototype.

## 📂 Repository Structure

The repository's branches serve different purposes. The structure below is a **combined view**, not a claim that every path exists on both branches.

```text
SPL-1/
├── learning/                 # Learning materials (main)
│   ├── 1-file-system-basics/
│   ├── 02-c-fundamentals/
│   ├── 03-bitwise-operations/
│   ├── 04-modular-programming/
│   ├── 05-file-handling/
│   ├── 06-binary-persistence/
│   ├── 07-virtual-disk-architecture/
│   └── 08-blocks-sectors-offsets/
├── FileSystem/               # Work-in-progress VFS (development)
│   ├── main.c
│   ├── disk.c
│   ├── superblock.c
│   ├── block.c
│   ├── file.c
│   └── fs.h
├── .gitignore
└── README.md                 # Overall project overview (main)
```

## 🚀 Build and Run

### Requirements

- GCC or another compatible C11 compiler
- Terminal (macOS, Linux, or Windows with a C toolchain)
- Git, to clone the source

### Clone and Build the Current Prototype

The implementation is maintained on the `development` branch:

```bash
git clone https://github.com/bsse1754-ridmi/SPL-1.git
cd SPL-1
git switch development
cd FileSystem
gcc -std=c11 -Wall -Wextra -Wpedantic main.c disk.c superblock.c block.c file.c -o vfs
./vfs
```

On Windows with MinGW, compile with `-o vfs.exe` and run `./vfs.exe` in a compatible shell.

**Note:** All five `.c` files are compiled together; `fs.h` is included by the source files. The program creates or opens `virtual_disk.bin` in its working directory. Run `format` for a new disk, but remember that **format erases any existing data**.

## 🖥️ CLI Demonstration

After launching `./vfs`, enter these commands:

```text
format
info
alloc
writeblock 37 Hello World
readblock 37
bitmap
map
stats
check
exit
```

On a newly formatted disk, `alloc` typically returns block `37`. After writing the text, restart `./vfs` and run `readblock 37` to demonstrate persistence. Do **not** reformat before checking the stored data.

### Available Storage Commands

| Command | Function |
|:--|:--|
| `help` | Show available commands |
| `format` | Initialize or erase the disk image |
| `info`, `status` | Display disk metadata and status |
| `alloc`, `allocat <block>` | Allocate a free data block |
| `free <block>`, `used <block>` | Release or check allocation |
| `writeblock <block> <text>` | Write text into an allocated block |
| `readblock <block>` | Read back block text |
| `copy <source> <target>` | Copy between allocated blocks |
| `compare <left> <right>` | Compare two allocated blocks |
| `inspect <block>`, `hex <block> [bytes]` | Inspect raw block content |
| `bitmap`, `map`, `stats` | Review block allocation and layout |
| `check`, `diagnostics` | Run metadata checks and diagnostics |
| `exit` | Exit the CLI |

Use `help` in the running program for the full current command list.

## 🛣️ Development Roadmap

### Phase 1 — Foundations and Learning

- [x] Study basic file-system concepts
- [x] Practice relevant C programming techniques
- [x] Explore binary storage and virtual-disk block addressing
- [x] Organize learning materials in the repository

### Phase 2 — Midterm: Storage Layer

- [x] Persistent virtual disk
- [x] Superblock and metadata handling
- [x] Bitmap-based allocation
- [x] Raw block read/write and inspection
- [x] Storage-level consistency checks
- [x] Interactive command-line controls

### Phase 3 — Full Virtual File System (Planned)

- [ ] Inode management and allocation
- [ ] Named file creation, writing, reading, and append
- [ ] Directory creation, listing, navigation, and removal
- [ ] Absolute and relative path resolution
- [ ] File deletion and metadata reporting
- [ ] Comprehensive tests and final integration

## 🌿 Git Branches

| Branch | Purpose |
|:--|:--|
| [`main`](https://github.com/bsse1754-ridmi/SPL-1/tree/main) | Main repository overview and learning materials |
| [`development`](https://github.com/bsse1754-ridmi/SPL-1/tree/development) | Active development of the Virtual File System |

> The default GitHub repository page displays the README from its default branch, normally `main`. The `development` branch can have different files and commits until changes are merged.

## 🔭 Future Direction

The current program establishes a foundation for understanding how data is stored and allocated. The next major milestone is to move from **addressing blocks by number** to **managing named files and directory structures**, while retaining the persistent virtual disk and storage-management concepts.

---

<div align="center">

**SPL-1 — Systems Programming Lab I**  
*Learning the foundations. Building the file system step by step.*

[GitHub Repository](https://github.com/bsse1754-ridmi/SPL-1)

</div>
