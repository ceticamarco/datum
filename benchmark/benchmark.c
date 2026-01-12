#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#include "../src/vector.h"
#include "../src/map.h"
#include "../src/string.h"

typedef void (*test_fn_t)(size_t iterations);

void test_vector(size_t iterations) {
    vector_t *vec = vector_new(16, sizeof(int)).value.vector;

    for (size_t idx = 0; idx < iterations; idx++) { 
        vector_push(vec, &(int){idx});
    }

    volatile uint64_t sum = 0;
    for (size_t idx = 0; idx < iterations; idx++) {
        const int *val = (int*)vector_get(vec, idx).value.element;
        sum += *val;
    }

    vector_destroy(vec);
}

void test_map(size_t iterations) {
    map_t *map = map_new().value.map;
    char key[64];

    for (size_t idx = 0; idx < iterations; idx++) {
        snprintf(key, sizeof(key), "key_%zu", idx);

        int *value = malloc(sizeof(int));
        *value = (int)idx;

        map_add(map, key, (void*)value);
    }

    volatile uint64_t sum = 0;
    for (size_t idx = 0; idx < iterations; idx++) {
        snprintf(key, sizeof(key), "key_%zu", idx);

        const int *val = (const int*)map_get(map, key).value.element;
        sum += *val;
    }

    // Cleanup values
    for (size_t idx = 0; idx < map->capacity; idx++) {
        snprintf(key, sizeof(key), "key_%zu", idx);

        int *val = (int*)map_get(map, key).value.element;
        free(val);
        
        map_remove(map, key);
    }

    map_destroy(map);
}

void test_string(size_t iterations) {
    volatile size_t total_len = 0;

    for (size_t idx = 0; idx < iterations; idx++) {
        string_t *str1 = string_new("hello").value.string;
        string_t *str2 = string_new(" World").value.string;

        string_result_t concat = string_concat(str1, str2);
        string_result_t upper = string_to_upper(concat.value.string);
        total_len += string_size(upper.value.string);
        string_result_t needle = string_new("WORLD");
        string_result_t contains = string_contains(upper.value.string, needle.value.string);

        if (contains.value.idx >= 0) {
            total_len += contains.value.idx;
        }

        string_destroy(str1);
        string_destroy(str2);
        string_destroy(concat.value.string);
        string_destroy(upper.value.string);
        string_destroy(needle.value.string);
    }
}

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

long long benchmark(test_fn_t fun, size_t iterations, size_t runs) {
    long long total = 0;
    
    for (size_t idx = 0; idx < runs; idx++) {
        uint64_t start = now_ns();
        fun(iterations);
        uint64_t end = now_ns();

        total += (end - start);
    }

    return (long long)(total / runs / 1000000);
}

int main(void) {
    // Do a warmup run
    test_vector(1000);
    test_map(1000);
    test_string(1000);

    printf("Computing Vector average time...");
    fflush(stdout);
    printf("average time: %lld ms\n", benchmark(test_vector, 1e6, 30));

    printf("Computing Map average time...");
    fflush(stdout);
    printf("average time: %lld ms\n", benchmark(test_map, 1e5, 30));

    printf("Computing String average time...");
    fflush(stdout);
    printf("average time: %lld ms\n", benchmark(test_string, 1e5, 30));

    return 0;
}
