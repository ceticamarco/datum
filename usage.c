/*
 * Sample usage of the Datum library.
 *
 * This program is a complete example on how to use Datum
 * with *verbose* error checking. For a more minimal usage, you may want to ignore
 * return messages/codes and get straight to the actual result. See the early
 * part of the README.md file for such example (use it at your own risk).
 *
 * Developed by Marco Cetica (c) 2025, <email@marcocetica.com>
 *
 */
#define SEP(SIZE) do { \
    for (size_t i = 0; i < SIZE; i++) { \
        printf("="); \
    }; \
    puts("\n"); \
} while(0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/vector.h"
#include "src/map.h"
#include "src/bigint.h"

static int vector_usage(void);
static int map_usage(void);
static int bigint_usage(void);

static vector_order_t cmp_int_asc(const void *x, const void *y);
static vector_order_t cmp_int_desc(const void *x, const void *y);

int main(void) {
    int st;

    st = vector_usage();
    if (st) { return st; }

    SEP(50);

    st = map_usage();
    if (st) { return st; }

    SEP(50);

    st = bigint_usage();
    if (st) { return st; }

    return 0;
}

vector_order_t cmp_int_asc(const void *x, const void *y) {
    int x_int = *(const int*)x;
    int y_int = *(const int*)y;

    if (x_int < y_int) return VECTOR_ORDER_LT;
    if (x_int > y_int) return VECTOR_ORDER_GT;

    return VECTOR_ORDER_EQ;
}

vector_order_t cmp_int_desc(const void *x, const void *y) {
    return cmp_int_asc(y, x);
}

int vector_usage(void) {
    // Create a vector of 3 integers
    vector_result_t res = vector_new(3, sizeof(int));
    if (res.status != VECTOR_OK) {
        printf("Error while creating vector: %s\n", res.message);

        return 1;
    }

    vector_t *vector = res.value.vector;

    // Push some values to trigger reallocation
    for (int idx = 0; idx < 5; idx++) {
        vector_result_t add_res = vector_push(vector, &idx);
        if (add_res.status != VECTOR_OK) {
            printf("Error while adding elements: %s\n", add_res.message);

            return 1;
        }
    }

    // Print vector size and capacity
    printf("Vector size (should be 5): %zu\n", vector_size(vector));
    printf("Vector capacity (should be > 5): %zu\n\n", vector_capacity(vector));

    // Print the whole vector
    size_t sz = vector_size(vector);
    for (size_t idx = 0; idx < sz; idx++) {
        vector_result_t get_res = vector_get(vector, idx);
        if (get_res.status != VECTOR_OK) {
            printf("Cannot retrieve vec[%zu]: %s\n", idx, get_res.message);

            return 1;
        } else {
            int val = *(int *)get_res.value.element;
            printf("vec[%zu] (should be '%zu') = %d\n", idx, idx, val);
        }
    }

    // Set an element at index 2
    int new_val = 0xBABE;
    vector_result_t set_res = vector_set(vector, 2, &new_val);
    if (set_res.status == VECTOR_OK) {
        printf("vec[2] (should be updated to 'BABE'): %X\n\n",  new_val);
    }

    // Pop last element
    vector_result_t pop_res = vector_pop(vector);
    if (pop_res.status == VECTOR_OK) {
        int val = *(int *)pop_res.value.element;
        printf("Popped value (should be 5) : %d\n\n", val);
    }

    // Clear vector
    vector_result_t clear_res = vector_clear(vector);
    if (clear_res.status != VECTOR_OK) {
        printf("Cannot clear vector: %s\n", clear_res.message);

        return 1;
    } else {
        printf("Vector cleared (size should be 0): %zu\n\n", vector_size(vector));
    }

    // Sort vector in ascending order
    int values[] = {5, 10, -9, 3, 1, 0, 4};
    for (size_t idx = 0; idx < 7; idx++) {
        vector_result_t sort_push_res = vector_push(vector, &values[idx]);
        if (sort_push_res.status != VECTOR_OK) {
            printf("Error while adding elements: %s\n", sort_push_res.message);

            return 1;
        }
    }

    printf("Added new elements. Before sort: ");

    sz = vector_size(vector);
    for (size_t idx = 0; idx < sz; idx++) {
        vector_result_t sort_get_res = vector_get(vector, idx);
        if (sort_get_res.status != VECTOR_OK) {
            printf("Cannot retrieve vec[%zu]: %s\n", idx, sort_get_res.message);

            return 1;
        } else {
            const int *val = (int*)sort_get_res.value.element;
            printf("%d ", *val);
        }
    }
    printf("\n");

    vector_result_t sort_asc_res = vector_sort(vector, cmp_int_asc);
    if (sort_asc_res.status != VECTOR_OK) {
        printf("Cannot sort array: %s\n", sort_asc_res.message);

        return 1;
    }

    printf("After sort in ascending order: ");
    for (size_t idx = 0; idx < sz; idx++) {
        vector_result_t sort_get_res = vector_get(vector, idx);
        if (sort_get_res.status != VECTOR_OK) {
            printf("Cannot retrieve vec[%zu]: %s\n", idx, sort_get_res.message);

            return 1;
        } else {
            int *val = (int*)sort_get_res.value.element;
            printf("%d ", *val);
        }
    }
    printf("\n");

    // Sort vector in descending order
    vector_result_t sort_desc_res = vector_sort(vector, cmp_int_desc);
    if (sort_desc_res.status != VECTOR_OK) {
        printf("Cannot sort array: %s\n", sort_desc_res.message);
        
        return 1;
    }

    printf("After sort in descending order: ");
    for (size_t idx = 0; idx < sz; idx++) {
        vector_result_t sort_get_res = vector_get(vector, idx);
        if (sort_get_res.status != VECTOR_OK) {
            printf("Cannot retrieve vec[%zu]: %s\n", idx, sort_get_res.message);

            return 1;
        } else {
            int *val = (int*)sort_get_res.value.element;
            printf("%d ", *val);
        }
    }
    printf("\n\n");

    // Free vector
    vector_result_t del_res = vector_destroy(vector);
    if (del_res.status != VECTOR_OK) {
        printf("Error while destroying the vector: %s\n", del_res.message);

        return 1;
    }

    return 0;
}

int map_usage(void) {
    // Create a new map
    map_result_t res = map_new();
    if (res.status != MAP_OK) {
        printf("Error while creating map: %s\n", res.message);

        return 1;
    }

    map_t *map = res.value.map;

    // Add some values
    const int x = 0xB00B5;
    const char *y = "Hello";

    map_result_t add_res = map_add(map, "x", (void*)&x);
    if (add_res.status != MAP_OK) {
        printf("Error while adding elements: %s\n", add_res.message);

        return 1;
    }

    add_res = map_add(map, "y", (void*)y);
    if (add_res.status != MAP_OK) {
        printf("Error while adding elements: %s\n", add_res.message);

        return 1;
    }

    // Print size and capacity
    printf("Map size (should be 2): %zu\n", map_size(map));
    printf("Map capacity (should be > 2): %zu\n\n", map_capacity(map));

    // Retrieve keys
    map_result_t get_res = map_get(map, "x");
    if (get_res.status != MAP_OK) {
        printf("Cannot retrieve map element 'x': %s\n", get_res.message);

        return 1;
    } else {
        const int *val = (const int*)get_res.value.element;
        printf("Key 'x' contains (should be 'B00B5'): %X\n", *val);
    }

    get_res = map_get(map, "y");
    if (get_res.status != MAP_OK) {
        printf("Cannot retrieve map element 'y': %s\n", get_res.message);

        return 1;
    } else {
        const char *val = (const char*)get_res.value.element;
        printf("Key 'y' contains (should be 'Hello') : %s\n\n", val);
    }

    // Update key
    const int new_x = 0xC0FFEE;
    map_result_t up_res = map_add(map, "x", (void*)&new_x);

    up_res = map_get(map, "x");
    if (get_res.status != MAP_OK) {
        printf("Cannot retrieve map element 'x': %s\n", get_res.message);

        return 1;
    } else {
        const int *val = (const int*)up_res.value.element;
        printf("Key 'x' (should be updated to 'C0FFEE'): %X\n\n", *val);
    }

    // Remove an element
    map_result_t rm_res = map_remove(map, "y");
    if (rm_res.status != MAP_OK) {
        printf("Cannot remove map element 'y': %s\n", rm_res.message);

        return 1;
    } else {
        printf("Map element 'y' removed (size should be 1): %zu\n\n", map_size(map));
    }

    // Clear the map
    map_result_t clear_res = map_clear(map);
    if (clear_res.status != MAP_OK) {
        printf("Cannot clear map: %s\n", clear_res.message);

        return 1;
    } else {
        printf("Map cleared (size should be 0): %zu\n", map_size(map));
    }

    printf("\n");

    // Delete the map
    map_result_t del_res = map_destroy(map);
    if (del_res.status != MAP_OK) {
        printf("Error while destroying the map: %s\n", del_res.message);

        return 1;
    }

    return 0;
}

int bigint_usage(void) {
    const char *x_origin = "8036732204560262312865077650774313136023641621894661847778962273940232785242208265819059749867858355";
    const char *y_origin = "7078840479830524979114102683681365071561983635405714511439038016617918064981439736383067887133445937";
    const size_t x_len = strlen(x_origin);
    const size_t y_len = strlen(y_origin);
    const size_t large_x_size = x_len * 100 + 1;
    const size_t large_y_size = y_len * 100 + 1;

    char *large_x = malloc(large_x_size);
    char *large_y = malloc(large_y_size);

    if (large_x == NULL || large_y == NULL) {
        printf("Error while allocating memory for strings\n");
        free(large_x);
        free(large_y);

        return 1;
    }

    large_x[0] = '\0';
    large_y[0] = '\0';

    // Concatenate 100 times
    for (size_t idx = 0; idx < 100; idx++) {
        strcat(large_x, x_origin);
        strcat(large_y, y_origin);
    }
    
    // Create two big integers from previous strings
    bigint_result_t x_res = bigint_from_string(large_x);
    if (x_res.status != BIGINT_OK) {
        printf("Error while creating big number: %s\n", x_res.message);

        return 1;
    }

    bigint_result_t y_res = bigint_from_string(large_y);
    if (x_res.status != BIGINT_OK) {
        printf("Error while creating big number: %s\n", x_res.message);

        return 1;
    }

    bigint_t *x = x_res.value.number;
    bigint_t *y = y_res.value.number;

    // Sum two big integers
    bigint_result_t sum_res = bigint_add(x, y);
    if (sum_res.status != BIGINT_OK) {
        printf("Error while summing two big numbers: %s\n", sum_res.message);

        return 1;
    }

    bigint_t *sum = sum_res.value.number;

    // Print result
    bigint_printf("Sum result = %B\n", sum);

    // Subtract two big integers
    bigint_result_t diff_res = bigint_sub(x, y);
    if (diff_res.status != BIGINT_OK) {
        printf("Error while subtracting two big numbers: %s\n", diff_res.message);

        return 1;
    }

    bigint_t *diff = diff_res.value.number;

    // Print result
    bigint_printf("difference result = %B\n", diff);

    // Multiply two big integers
    bigint_result_t prod_res = bigint_prod(x, y);
    if (prod_res.status != BIGINT_OK) {
        printf("Error while multiplying two big numbers: %s\n", prod_res.message);

        return 1;
    }

    bigint_t *prod = prod_res.value.number;

    // Print result
    bigint_printf("multiplication result = %B\n", prod);

    bigint_t *a = bigint_from_string(x_origin).value.number;
    bigint_t *b = bigint_from_string(y_origin).value.number;

    // Divide two big integers
    bigint_result_t div_res = bigint_divmod(a, b);
    if (div_res.status != BIGINT_OK) {
        printf("Error while dividing two big numbers: %s\n", div_res.message);

        return 1;
    }

    bigint_t *quotient = div_res.value.division.quotient;
    bigint_t *remainder = div_res.value.division.remainder;

    // Print result
    bigint_printf(
    "division result = %B\
    \nmod result = %B\n",
        quotient, remainder);

    // Destroy big numbers and strings
    bigint_destroy(x); bigint_destroy(y);
    bigint_destroy(a); bigint_destroy(b);
    bigint_destroy(sum); bigint_destroy(diff); 
    bigint_destroy(prod); bigint_destroy(quotient); bigint_destroy(remainder);
    free(large_x); free(large_y);

    return 0;
}
