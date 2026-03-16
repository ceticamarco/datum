CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic-errors -fstack-protector-strong \
	-fsanitize=address -fsanitize=undefined -fstack-clash-protection \
	-Wwrite-strings -g -std=c99

SRC_DIR = ../src
OBJ_DIR = ../obj

TARGETS = vector_basic vector_sorting vector_functional map_basic bigint_operations string_basic string_advanced

.PHONY: all clean

all: $(TARGETS)

vector_basic: vector_basic.c $(OBJ_DIR)/vector.o
	$(CC) $(CFLAGS) -o $@ $^

vector_sorting: vector_sorting.c $(OBJ_DIR)/vector.o
	$(CC) $(CFLAGS) -o $@ $^

vector_functional: vector_functional.c $(OBJ_DIR)/vector.o
	$(CC) $(CFLAGS) -o $@ $^

map_basic: map_basic.c $(OBJ_DIR)/map.o
	$(CC) $(CFLAGS) -o $@ $^

bigint_operations: bigint_operations.c $(OBJ_DIR)/bigint.o $(OBJ_DIR)/vector.o
	$(CC) $(CFLAGS) -o $@ $^

string_basic: string_basic.c $(OBJ_DIR)/string.o
	$(CC) $(CFLAGS) -o $@ $^

string_advanced: string_advanced.c $(OBJ_DIR)/string.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)