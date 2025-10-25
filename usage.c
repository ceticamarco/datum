#include <stdio.h>
#include "src/vector.h"

int main(void) {
    // Create a vector of 5 integers
    vector_result_t res = vector_new(5, sizeof(int));
    if (res.status != VECTOR_OK) {
        printf("Error while creating vector: %s\n", res.message);

        return 1;
    }

    vector_t *vector = res.value.vector;

    // Push some values
    for (int idx = 0; idx <= 5; idx++) {
        vector_result_t add_res = vector_push(vector, &idx);
        if (add_res.status != VECTOR_OK) {
            printf("Error while adding elements: %s\n", add_res.message);

            return 1;
        }
    }

    // Print vector size and capacity
    printf("Vector size: %zu\n", vector_size(vector));
    printf("Vector capacity: %zu\n", vector_capacity(vector));

    // Print the whole vector
    for (size_t idx = 0; idx < vector_size(vector); idx++) {
        vector_result_t get_res = vector_get(vector, idx);
        if (get_res.status == VECTOR_OK) {
            int val = *(int *)get_res.value.element;
            printf("vec[%zu] = %d\n", idx, val);
        }
    }

    // Set an element at index 2
    int new_val = 0xABABA;
    vector_result_t set_res = vector_set(vector, 2, &new_val);
    if (set_res.status == VECTOR_OK) {
        printf("vec[2] updated to %d\n", new_val);
    }

    // Pop last element
    vector_result_t pop_res = vector_pop(vector);
    if (pop_res.status == VECTOR_OK) {
        int val = *(int *)pop_res.value.element;
        printf("Popped value: %d\n", val);
    }

    // Clear vector
    vector_result_t clear_res = vector_clear(vector);
    if (clear_res.status == VECTOR_OK) {
        printf("Vector cleared. New size is: %zu\n", vector_size(vector));
    }

    // Free vector
    vector_result_t free_res = vector_free(vector);
    if (free_res.status != VECTOR_OK) {
        printf("Error while freeing the vector: %s\n", free_res.message);
    }

    return 0;
}
