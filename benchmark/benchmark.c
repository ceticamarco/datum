#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#include "../src/vector.h"
#include "../src/map.h"

typedef void (*test_fn_t)(size_t iterations);

void test_vector(size_t iterations) {
    vector_t *vec = vector_new(16, sizeof(int)).value.vector;

    for (size_t idx = 0; idx < iterations; idx++) { 
        vector_push(vec, &idx); 
    }

    volatile uint64_t sum = 0; // prevent the compiler from optimizing away the sum
    for (size_t idx = 0; idx < iterations; idx++) {
        const int *val = (int*)vector_get(vec, idx).value.element;
        sum += *val;
    }

    // Another trick to prevent compiler optimization
    if (sum == 0xB00B5) {
        printf("sum = %llu\n", (unsigned long long)sum);
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

    volatile uint64_t sum = 0; // prevent the compiler from optimizing away the sum
    for (size_t idx = 0; idx < iterations; idx++) {
        snprintf(key, sizeof(key), "key_%zu", idx);

        const int *val = (const int*)map_get(map, key).value.element;
        sum += *val;
    }

    // Cleanup values
    for (size_t idx = 0; idx < map->capacity; idx++) {
        if (map->elements[idx].state == ENTRY_OCCUPIED) {
            int *val = (int*)map->elements[idx].value;
            free(val);
        }
    }

    map_destroy(map);
}

long long benchmark(test_fn_t fun, size_t iterations, size_t runs) {
    long long total = 0;
    for (size_t idx = 0; idx < runs; idx++) {
        clock_t start = clock();
        fun(iterations);
        clock_t end = clock();

        total += (long long)((end - start) * 1000 / CLOCKS_PER_SEC);
    }

    return total / runs;
}

int main(void) {
    // Do a warmup run
    test_vector(1000);
    test_map(1000);

    printf("Computing Vector average time...");
    fflush(stdout);
    printf("average time: %lld ms\n", benchmark(test_vector, 1e6, 30));

    printf("Computing Map average time...");
    fflush(stdout);
    printf("average time: %lld ms\n", benchmark(test_map, 1e5, 30));

    return 0;
}