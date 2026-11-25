CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2

all: bitmap_demo bitmap_persist

bitmap_demo: bitmap_demo.c bitmap.c bitmap.h
	$(CC) $(CFLAGS) bitmap_demo.c bitmap.c -o $@

bitmap_persist: bitmap_persist.c bitmap.c bitmap.h
	$(CC) $(CFLAGS) bitmap_persist.c bitmap.c -o $@

test: all
	./bitmap_demo
	./bitmap_persist init
	./bitmap_persist alloc
	./bitmap_persist show
	./bitmap_persist free 11
	./bitmap_persist show

clean:
	rm -f bitmap_demo bitmap_persist learning11_bitmap.bin

.PHONY: all test clean
