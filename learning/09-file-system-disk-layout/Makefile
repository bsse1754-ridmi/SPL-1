CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

all: layout_demo disk_layout_image

layout_demo: layout_demo.c
	$(CC) $(CFLAGS) $< -o $@

disk_layout_image: disk_layout_image.c
	$(CC) $(CFLAGS) $< -o $@

test: all
	./layout_demo
	./disk_layout_image create
	./disk_layout_image inspect

clean:
	rm -f layout_demo disk_layout_image learning09_disk.bin

.PHONY: all test clean
