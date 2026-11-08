CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

all: virtual_disk_demo

virtual_disk_demo: virtual_disk_demo.c
	$(CC) $(CFLAGS) virtual_disk_demo.c -o virtual_disk_demo

run: virtual_disk_demo
	./virtual_disk_demo init
	./virtual_disk_demo info
	./virtual_disk_demo write 3 "Hello from Learning 07"
	./virtual_disk_demo read 3

test: virtual_disk_demo
	./virtual_disk_demo init
	./virtual_disk_demo write 3 "Persistent block data"
	./virtual_disk_demo read 3
	./virtual_disk_demo zero 3
	./virtual_disk_demo read 3
	@if ./virtual_disk_demo read 64 >/dev/null 2>&1; then echo "FAIL: invalid block accepted"; exit 1; else echo "PASS: invalid block rejected"; fi

clean:
	rm -f virtual_disk_demo learning07_disk.bin

.PHONY: all run test clean
