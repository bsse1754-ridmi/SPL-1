CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
OBJECTS = main.o bitmap.o report.o
TARGET = modular_demo

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c bitmap.h report.h
	$(CC) $(CFLAGS) -c main.c -o main.o

bitmap.o: bitmap.c bitmap.h
	$(CC) $(CFLAGS) -c bitmap.c -o bitmap.o

report.o: report.c report.h bitmap.h
	$(CC) $(CFLAGS) -c report.c -o report.o

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
