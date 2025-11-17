CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic-errors -fstack-protector-strong \
	-fsanitize=address -fsanitize=undefined -fstack-clash-protection \
	-Wwrite-strings -g -std=c99

SRC_DIR = src
OBJ_DIR = obj
TESTS_SRC = tests

TARGET = usage
TEST_V_TARGET = test_vector
TEST_M_TARGET = test_map
TEST_B_TARGET = test_bigint

LIB_OBJS = $(OBJ_DIR)/vector.o $(OBJ_DIR)/map.o $(OBJ_DIR)/bigint.o
PROG_OBJS = $(OBJ_DIR)/usage.o

.PHONY: all clean

all: $(TARGET) $(TEST_V_TARGET) $(TEST_M_TARGET) $(TEST_B_TARGET)

$(TARGET): $(PROG_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_V_TARGET): $(OBJ_DIR)/test_vector.o $(OBJ_DIR)/vector.o
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_M_TARGET): $(OBJ_DIR)/test_map.o $(OBJ_DIR)/map.o
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_B_TARGET): $(OBJ_DIR)/test_bigint.o $(OBJ_DIR)/bigint.o $(OBJ_DIR)/vector.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/usage.o: usage.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(TESTS_SRC)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TEST_V_TARGET) $(TEST_M_TARGET) $(TEST_B_TARGET)
