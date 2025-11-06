/*
 * Unit tests for Map data type
 */

#define TEST(NAME) do { \
    printf("Running test_%s...", #NAME); \
    test_##NAME(); \
    printf(" PASSED\n"); \
} while(0)

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../src/map.h"

// Create a new map
void test_map_new() {
    map_result_t res = map_new();

    assert(res.status == MAP_OK);
    assert(res.value.map != NULL);
    assert(map_size(res.value.map) == 0);
    assert(map_capacity(res.value.map) > 0);

    map_destroy(res.value.map);
}

// Add elements to map
void test_map_add() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    const int x = 42, y = 84;
    
    map_result_t x_res = map_add(map, "key1", (void*)&x);
    assert(x_res.status == MAP_OK);
    assert(map_size(map) == 1);

    map_result_t y_res = map_add(map, "key2", (void*)&y);
    assert(y_res.status == MAP_OK);
    assert(map_size(map) == 2);

    map_destroy(map);
}

// Add multiple elements to the map
void test_map_add_multiple() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    const int x = 0xB00B5;
    const char *y = "Hello";

    map_result_t add_res = map_add(map, "x", (void*)&x);
    assert(add_res.status == MAP_OK);

    add_res = map_add(map, "y", (void*)y);
    assert(add_res.status == MAP_OK);

    assert(map_size(map) == 2);

    map_destroy(map);
}

// Get map element
void test_map_get() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    const int val = 123;
    map_add(map, "test", (void*)&val);

    map_result_t get_res = map_get(map, "test");
    assert(get_res.status == MAP_OK);
    assert(*(int*)get_res.value.element == 123);

    map_destroy(map);
}

// Get non-existing key from map
void test_map_get_invalid() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    map_result_t get_res = map_get(map, "boom");
    assert(get_res.status == MAP_ERR_NOT_FOUND);

    map_destroy(map);
}

// Map with heterogeneous types
void test_map_mixed() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    const int x = 0xB00B5;
    const char *y = "Hello";

    map_add(map, "x", (void*)&x);
    map_add(map, "y", (void*)y);

    map_result_t get_res = map_get(map, "x");
    assert(get_res.status == MAP_OK);

    const int *val_x = (const int*)get_res.value.element;
    assert(*val_x == 0xB00B5);

    get_res = map_get(map, "y");
    assert(get_res.status == MAP_OK);

    const char *val_y = (const char*)get_res.value.element;
    assert(!strcmp(val_y, "Hello"));

    map_destroy(map);
}

// Update existing map key
void test_map_update() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    const int x = 100, y = 200;

    map_add(map, "key", (void*)&x);

    map_result_t set_res = map_add(map, "key", (void*)&y);
    assert(set_res.status == MAP_OK);

    map_result_t get_res = map_get(map, "key");
    const int *val = (const int*)get_res.value.element;

    assert(*val == 200);
    assert(map_size(map) == 1);

    map_destroy(map);
}

// Remove an element from map
void test_map_remove() {
    map_result_t res = map_new();

    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    const int x = 10, y = 20;

    map_add(map, "x", (void*)&x);
    map_add(map, "y", (void*)&y);

    assert(map_size(map) == 2);

    map_result_t rm_res = map_remove(map, "x");
    assert(rm_res.status == MAP_OK);
    assert(map_size(map) == 1);

    // Check whether the 'x' and 'y' keys are still there
    map_result_t get_res = map_get(map, "x");
    assert(get_res.status != MAP_OK);

    get_res = map_get(map, "y");
    assert(get_res.status == MAP_OK);

    map_destroy(map);
}

// Remove non-existing key from map
void test_map_remove_invalid() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    map_result_t rm_res = map_remove(map, "boom");
    assert(rm_res.status != MAP_OK);

    map_destroy(map);
}

// Clear the map
void test_map_clear() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    const int x = 10, y = 20, z = 30;

    map_add(map, "x", (void*)&x);
    map_add(map, "y", (void*)&y);
    map_add(map, "z", (void*)&z);
    
    assert(map_size(map) == 3);

    map_result_t clear_res = map_clear(map);
    assert(clear_res.status == MAP_OK);
    assert(map_size(map) == 0);

    map_destroy(map);
}

// Clear empty map
void test_map_clear_empty() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    map_result_t clear_res = map_clear(map);
    assert(clear_res.status == MAP_OK);
    assert(map_size(map) == 0);

    map_destroy(map);
}

// Multiple operations in sequence (add, update, delete and clear)
void test_map_sequence() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    const int x = 0xB00B5;
    const char *y = "Hello";

    // Add
    map_add(map, "x", (void*)&x);
    map_add(map, "y", (void*)&y);

    assert(map_size(map) == 2);

    // Update
    const int new_x = 0xC0FFEE;
    map_add(map, "x", (void*)&new_x);

    map_result_t get_res = map_get(map, "x");
    assert(*(const int*)get_res.value.element == 0xC0FFEE);

    // Delete
    map_remove(map, "y");
    assert(map_size(map) == 1);

    // Clear
    map_clear(map);
    assert(map_size(map) == 0);

    map_destroy(map);
}

// Test map with product data types
typedef struct {
    char name[256];
    char surname[256];
    short age;
} Person;

void test_map_struct() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    const Person bob = { "Bob", "Miller", 23 };
    const Person alice = { "Alice", "Davis", 21 };

    map_add(map, "af94rt", (void*)&bob);
    map_add(map, "b910o5", (void*)&alice);

    map_result_t get_res = map_get(map, "af94rt");
    assert(get_res.status == MAP_OK);

    const Person *retr = (const Person*)get_res.value.element;
    assert(!strcmp(retr->name, "Bob"));
    assert(!strcmp(retr->surname, "Miller"));
    assert(retr->age == 23);

    get_res = map_get(map, "b910o5");
    assert(get_res.status == MAP_OK);

    retr = (const Person*)get_res.value.element;
    assert(!strcmp(retr->name, "Alice"));
    assert(!strcmp(retr->surname, "Davis"));
    assert(retr->age == 21);

    map_destroy(map);
}

// Test map capacity tracking
void test_map_cap() {
    map_result_t res = map_new();
    
    assert(res.status == MAP_OK);
    map_t *map = res.value.map;

    for (int i = 0; i < 10; i++) {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        map_add(map, key, &i);
    }

    assert(map_size(map) == 10);
    assert(map_capacity(map) >= 10);

    map_destroy(map);
}

int main(void) {
    printf("=== Running Map unit tests ===\n\n");

    TEST(map_new);
    TEST(map_add);
    TEST(map_add_multiple);
    TEST(map_get);
    TEST(map_get_invalid);
    TEST(map_mixed);
    TEST(map_update);
    TEST(map_remove);
    TEST(map_remove_invalid);
    TEST(map_clear);
    TEST(map_clear_empty);
    TEST(map_sequence);
    TEST(map_struct);
    TEST(map_cap);

    printf("\n=== All tests passed! ===\n");

    return 0;
}
