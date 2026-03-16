/*
 * Vector sorting example.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/vector.h"

static vector_order_t cmp_int_asc(const void *x, const void *y);
static vector_order_t cmp_int_desc(const void *x, const void *y);

int main(void) {
    // Create a vector
    vector_result_t res = vector_new(1, sizeof(int));
    if (res.status != VECTOR_OK) {
        printf("Error while creating vector: %s\n", res.message);
        return 1;
    }

    vector_t *vector = res.value.vector;

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

    size_t sz = vector_size(vector);
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

