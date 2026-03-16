/*
 * Basic vector operations example.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/vector.h"

int main(void) {
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

    // Free vector
    vector_result_t del_res = vector_destroy(vector);
    if (del_res.status != VECTOR_OK) {
        printf("Error while destroying the vector: %s\n", del_res.message);
        return 1;
    }

    return 0;
}

