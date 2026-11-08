CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2

all: block_span_demo sector_mapping_demo

block_span_demo: block_span_demo.c
	$(CC) $(CFLAGS) $< -o $@

sector_mapping_demo: sector_mapping_demo.c
	$(CC) $(CFLAGS) $< -o $@

test: all
	./block_span_demo
	./sector_mapping_demo

clean:
	rm -f block_span_demo sector_mapping_demo learning08_disk.bin

.PHONY: all test clean
