#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#include "../src/vector.h"
#include "../src/map.h"
#include "../src/bigint.h"
#include "../src/string.h"

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
        snprintf(key, sizeof(key), "key_%zu", idx);

        int *val = (int *)map_get(map, key).value.element;
        free(val);

        map_remove(map, key);
    }

    map_destroy(map);
}

void test_bigint(size_t iterations) {
    volatile uint64_t accumulator = 0;

    for (size_t idx = 1; idx <= iterations; idx++) {
        long long a_val = (long long)idx * 123456789LL;
        long long b_val = (long long)idx * 17777LL;

        bigint_result_t a_res = bigint_from_int(a_val);
        bigint_result_t b_res = bigint_from_int(b_val);

        if (a_res.status != BIGINT_OK || b_res.status != BIGINT_OK) {
            bigint_destroy(a_res.value.number);
            bigint_destroy(b_res.value.number);
            continue;
        }

        bigint_t *a = a_res.value.number;
        bigint_t *b = b_res.value.number;

        // Addition
        bigint_result_t add_res = bigint_add(a, b);
        if (add_res.status == BIGINT_OK) {
            vector_result_t v = vector_get(add_res.value.number->digits, 0);
            if (v.status == VECTOR_OK) { accumulator += *(int *)v.value.element; }
            bigint_destroy(add_res.value.number);
        }

        // Substraction
        bigint_result_t sub_res = bigint_sub(a, b);
        if (sub_res.status == BIGINT_OK) {
            vector_result_t v = vector_get(sub_res.value.number->digits, 0);
            if (v.status == VECTOR_OK) { accumulator += *(int *)v.value.element; }
            bigint_destroy(sub_res.value.number);
        }

        // Multiplication
        bigint_result_t mul_res = bigint_prod(a, b);
        if (mul_res.status == BIGINT_OK) {
            vector_result_t v = vector_get(mul_res.value.number->digits, 0);
            if (v.status == VECTOR_OK) { accumulator += *(int *)v.value.element; }
            bigint_destroy(mul_res.value.number);
        }

        // Division
        bigint_result_t div_res = bigint_divmod(a, b);
        if (div_res.status == BIGINT_OK) {
            vector_result_t v = vector_get(div_res.value.division.quotient->digits, 0);
            if (v.status == VECTOR_OK) { accumulator += *(int *)v.value.element; }
            bigint_destroy(div_res.value.division.quotient);
            bigint_destroy(div_res.value.division.remainder);
        }

        bigint_destroy(a); bigint_destroy(b);
    }
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
    test_bigint(1000);

    printf("Computing Vector average time...");
    fflush(stdout);
    printf("average time: %lld ms\n", benchmark(test_vector, 1e6, 30));

    printf("Computing Map average time...");
    fflush(stdout);
    printf("average time: %lld ms\n", benchmark(test_map, 1e5, 30));

    printf("Computing BigInt average time...");
    fflush(stdout);
    printf("average time: %lld ms\n", benchmark(test_bigint, 1e5, 30));

    printf("Computing String average time...");
    fflush(stdout);
    printf("average time: %lld ms\n", benchmark(test_string, 1e5, 30));

    return 0;
}
