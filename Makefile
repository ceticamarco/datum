CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic-errors -fstack-protector-strong \
	-fsanitize=address -fsanitize=undefined -fstack-clash-protection \
	-Wwrite-strings -std=c99

SRC_DIR = src
OBJ_DIR = obj
TESTS_SRC = tests

TARGET = usage
TEST_TARGET = test_vector

LIB_OBJS = $(OBJ_DIR)/vector.o $(OBJ_DIR)/map.o
PROG_OBJS = $(OBJ_DIR)/usage.o
TESTS_OBJS = $(OBJ_DIR)/test_vector.o

.PHONY: all clean

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(PROG_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_TARGET): $(TESTS_OBJS) $(OBJ_DIR)/vector.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(TESTS_SRC)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TEST_TARGET)
