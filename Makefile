CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic-errors -fstack-protector-strong \
	-fsanitize=address -fsanitize=undefined -fstack-clash-protection \
	-Wwrite-strings -std=c99
LDFLAGS = -fsanitize=address -fsanitize=undefined

SRC_DIR = src
OBJ_DIR = obj

TARGET = usage
LIB_OBJS = $(OBJ_DIR)/vector.o $(OBJ_DIR)/map.o
PROG_OBJS = $(OBJ_DIR)/usage.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(PROG_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
