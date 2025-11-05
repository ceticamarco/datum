/*
 * Unit tests for Vector
*/

#define TEST(NAME) do { \
    printf("Running test_%s...", #NAME); \
    test_##NAME(); \
    printf(" PASSED\n"); \
} while(0)

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../src/vector.h"

// Create a new vector
void test_vector_new() {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    assert(res.value.vector != NULL);
    assert(vector_size(res.value.vector) == 0);
    assert(vector_capacity(res.value.vector) == 5);

    vector_destroy(res.value.vector);
}

// Create a vector with zero capacity
void test_vector_new_zcap() {
    vector_result_t res = vector_new(0, sizeof(int));

    assert(res.status == VECTOR_ERR_ALLOCATE);
}

// Push elements to vector
void test_vector_push() {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int x = 42, y = 84;

    vector_result_t x_res = vector_push(v, &x);
    assert(x_res.status == VECTOR_OK);
    assert(vector_size(v) == 1);

    vector_result_t y_res = vector_push(v, &y);
    assert(y_res.status == VECTOR_OK);
    assert(vector_size(v) == 1);

    vector_destroy(v);
}

// Trigger vector reallocation
void test_vector_push_realloc() {
    vector_result_t res = vector_new(2, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    for (int i = 0; i < 5; i++) {
        vector_result_t push_res = vector_push(v, &i);
        assert(push_res.status == VECTOR_OK);
    }

    assert(vector_size(v) == 5);
    assert(vector_capacity(v) >= 5);

    vector_destroy(v);
}

// Get vector elements
void test_vector_get() {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int val = 123;
    vector_push(v, &val);

    vector_result_t get_res = vector_get(v, 0);
    assert(get_res.status == VECTOR_OK);
    assert(*(int*)get_res.value.element == 123);

    vector_destroy(v);
}

// Test out of bounds
void test_vector_get_ofb() {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int val = 123;
    vector_push(v, &val);

    vector_result_t get_res = vector_get(v, 10);
    assert(get_res.status != VECTOR_OK);

    vector_destroy(v);
}

// Set vector element
void test_vector_set() {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int x = 123, y = 999;
    vector_push(v, &x);

    vector_result_t set_res = vector_set(v, 0, &y);
    assert(set_res.status == VECTOR_OK);

    vector_result_t get_res = vector_get(v, 0);
    assert(*(int*)get_res.value.element == 999);

    vector_destroy(v);
}

// Set vector elemement out of bounds
void test_vector_set_ofb() {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int x = 10, y = 999;
    vector_push(v, &x);

    vector_result_t set_res = vector_set(v, 10, &y);
    assert(set_res.status != VECTOR_OK);

    vector_destroy(v);
}

// Pop element from vector
void test_vector_pop() {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int x = 10, y = 20;
    vector_push(v, &x);
    vector_push(v, &y);

    assert(vector_size(v) == 2);

    vector_result_t pop_res = vector_pop(v);
    assert(pop_res.status == VECTOR_OK);
    assert(*(int*)pop_res.value.element == 20);
    assert(vector_size(v) == 1);

    vector_destroy(v);
}

// Test pop element from empty vector
void test_vector_pop_empty() {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    vector_result_t pop_res = vector_pop(v);
    assert(pop_res.status != VECTOR_OK);

    vector_destroy(v);
}

// Clear vector
void test_vector_clear() {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    for (int i = 0; i < 5; i++) {
        vector_push(v, &i);
    }

    assert(vector_size(v) == 5);

    vector_result_t clear_res = vector_clear(v);
    assert(clear_res.status == VECTOR_OK);
    assert(vector_size(v) == 0);
    // Capacity should be unchanged, this is by design!
    assert(vector_capacity(v) == 5);

    vector_destroy(v);
}

// Multiple operations in sequence (push, set, pop and clear)
void test_vector_sequence() {
    vector_result_t res = vector_new(2, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    // push
    for (int i = 0; i < 5; i++) {
        vector_push(v, &i);
    }

    // Set
    int new_val = 0xBABE;
    vector_set(v, 2, &new_val);

    vector_result_t get_res = vector_get(v, 2);
    assert(*(int*)get_res.value.element == 0xBABE);

    // Pop
    vector_pop(v);
    assert(vector_size(v) == 4);

    // Clear
    vector_clear(v);
    assert(vector_size(v) == 0);

    vector_destroy(v);
}

// Vector with chars
void test_vector_char() {
    vector_result_t res = vector_new(5, sizeof(char));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    char x = 'A', y = 'B', z = 'C';
    vector_push(v, &x);
    vector_push(v, &y);
    vector_push(v, &z);

    vector_result_t get_res = vector_get(v, 1);
    assert(*(char *)get_res.value.element == 'B');

    vector_destroy(v);
}

// Test vector with product data type
typedef struct {
    int x;
    int y;
} Point;

void test_vector_struct() {
    vector_result_t res = vector_new(5, sizeof(Point));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;
   
    Point p1 = {10, 20};
    Point p2 = {30, 40};

    vector_push(v, &p1);
    vector_push(v, &p2);

    vector_result_t get_res = vector_get(v, 0);
    Point *retr = (Point*)get_res.value.element;
    assert(retr->x == 10);
    assert(retr->y == 20);

    vector_destroy(v);
}

int main(void) {
    printf("=== Running Vector unit tests ===\n\n");

    TEST(vector_new);
    TEST(vector_new_zcap);
    TEST(vector_push_realloc);
    TEST(vector_get);
    TEST(vector_get_ofb);
    TEST(vector_set);
    TEST(vector_set_ofb);
    TEST(vector_pop);
    TEST(vector_pop_empty);
    TEST(vector_clear);
    TEST(vector_sequence);
    TEST(vector_char);
    TEST(vector_struct);

    printf("\n=== All tests passed! ===\n");
    
    return 0;
}
