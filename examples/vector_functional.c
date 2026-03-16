/*
 * Vector functional operations example.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/vector.h"

#define UNUSED(X) (void)(X)

static void square(void *element, void *env);
static int is_even(const void *element, void *env);
static void adder(void *accumulator, const void *element, void *env);

int main(void) {
    // Create a vector
    vector_result_t res = vector_new(1, sizeof(int));
    if (res.status != VECTOR_OK) {
        printf("Error while creating vector: %s\n", res.message);
        return 1;
    }

    vector_t *vector = res.value.vector;

    // Map vector elements
    for (size_t idx = 1; idx <= 5; idx++) {
        vector_result_t map_push_res = vector_push(vector, &idx);
        if (map_push_res.status != VECTOR_OK) {
            printf("Error while adding elements: %s\n", map_push_res.message);
            return 1;
        }
    }

    size_t sz = vector_size(vector);

    // Square vector elements: [1, 2, 3, 4, 5] -> [1, 4, 9, 16, 25]
    vector_result_t map_res = vector_map(vector, square, NULL);
    if (map_res.status != VECTOR_OK) {
        printf("Error while mapping vector: %s\n", map_res.message);
        return 1;
    }

    printf("Squared vector: ");
    for (size_t idx = 0; idx < sz; idx++) {
        vector_result_t map_get_res = vector_get(vector, idx);
        if (map_get_res.status != VECTOR_OK) {
            printf("Cannot retrieve vec[%zu]: %s\n", idx, map_get_res.message);
            return 1;
        } else {
            int *val = (int*)map_get_res.value.element;
            printf("%d ", *val);
        }
    }

    printf("\n");

    // Filter vector elements: [1, 4, 9, 16, 25] -> [4, 16]
    vector_result_t filter_res = vector_filter(vector, is_even, NULL);
    if (filter_res.status != VECTOR_OK) {
        printf("Error while filtering vector: %s\n", filter_res.message);
        return 1;
    }

    sz = vector_size(vector);

    printf("Filtered vector: ");
    for (size_t idx = 0; idx < sz; idx++) {
        vector_result_t map_get_res = vector_get(vector, idx);
        if (map_get_res.status != VECTOR_OK) {
            printf("Cannot retrieve vec[%zu]: %s\n", idx, map_get_res.message);
            return 1;
        } else {
            int *val = (int*)map_get_res.value.element;
            printf("%d ", *val);
        }
    }

    printf("\n");

    // Reduce vector elements: [4, 16] -> 20
    int sum = 0;
    vector_result_t reduce_res = vector_reduce(vector, &sum, adder, NULL);
    if (reduce_res.status != VECTOR_OK) {
        printf("Error while reducing vector: %s\n", reduce_res.message);
        return 1;
    }

    printf("Sum of vector: %d\n", sum);

    // Free vector
    vector_result_t del_res = vector_destroy(vector);
    if (del_res.status != VECTOR_OK) {
        printf("Error while destroying the vector: %s\n", del_res.message);
        return 1;
    }

    return 0;
}

void square(void *element, void *env) {
    UNUSED(env);
    int *value = (int*)element;
    *value = (*value) * (*value);
}

int is_even(const void *element, void *env) {
    UNUSED(env);
    int value = *(int*)element;

    return (value % 2) == 0;
}

void adder(void *accumulator, const void *element, void *env) {
    UNUSED(env);
    *(int*)accumulator += *(int*)element;
}

