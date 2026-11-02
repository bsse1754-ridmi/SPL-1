# Learning 01 - File System Fundamentals

This first lesson introduces how operating systems organize files and directories, and connects the ideas to basic **C11 file I/O**.

## Learning outcomes

- Explain files, directories, paths, metadata, and the role of a file system.
- Distinguish a *host OS file system* from the *custom file system simulator* built later.
- Understand the difference between logical file names and physical storage blocks.
- Build and run a small C program that creates, reads, appends, and deletes a text file.

## Files

- `notes.pdf` - Illustrated learning guide, examples, glossary, mini-quiz, and limitations.
- `fs_intro_demo.c` - Standalone, defensive C11 file-I/O demonstration.

## Run the demo

Run these commands **from this folder**:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 fs_intro_demo.c -o fs_intro_demo
./fs_intro_demo
```

The program creates `fs_intro_sample.txt` in your current working directory, prints its initial contents, appends another line, and prints again. It deliberately refuses to overwrite the sample file if it already exists.

To remove the generated sample file:

```bash
./fs_intro_demo --clean
```

On Windows Command Prompt/PowerShell, run `fs_intro_demo.exe` (and `fs_intro_demo.exe --clean`) instead of `./fs_intro_demo`.

## Expected output

```text
1. Created a file and wrote the first line.
Contents of fs_intro_sample.txt:
Hello from a C file!
2. Appended another line.
Contents of fs_intro_sample.txt:
Hello from a C file!
This line was appended.
3. The file remains on disk as fs_intro_sample.txt.
4. Run the program with --clean to remove the demo file.
```

## Important distinction

This demonstration calls standard C library routines (`fopen`, `fputs`, `fgets`, `fclose`, `remove`). It **does not implement its own disk blocks, inodes, or directory structures**. Those are future learning modules and parts of the main simulator.

## Next

Learning 02 - Disk and Block Management.
