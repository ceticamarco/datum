/*
 * Unit tests for BigInt data type
*/

#define TEST(NAME) do { \
    printf("Running test_%s...", #NAME); \
    test_##NAME(); \
    printf(" PASSED\n"); \
} while(0)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/bigint.h"

static void bigint_eq(const bigint_t *number, const char *expected) {
    bigint_result_t to_str = bigint_to_string(number);
    assert(to_str.status == BIGINT_OK);
    assert(!strcmp(to_str.value.string_num, expected));
    
    free(to_str.value.string_num);
}

// Test creating big integers from int
void test_bigint_from_int() {
    bigint_result_t res = bigint_from_int(0);
    
    assert(res.status == BIGINT_OK);
    bigint_eq(res.value.number, "0");
    bigint_destroy(res.value.number);

    res = bigint_from_int(10);
    assert(res.status == BIGINT_OK);
    bigint_eq(res.value.number, "10");
    bigint_destroy(res.value.number);

    res = bigint_from_int(-12345678900LL);
    assert(res.status == BIGINT_OK);
    bigint_eq(res.value.number, "-12345678900");
    bigint_destroy(res.value.number);
}

// Test creating big integers from string
void test_bigint_from_string() {
    bigint_result_t res = bigint_from_string("00000123");

    assert(res.status == BIGINT_OK);
    bigint_eq(res.value.number, "123");
    bigint_destroy(res.value.number);
}

int main(void) {
    printf("=== Running BigInt unit tests ===\n\n");

    TEST(bigint_from_int);
    TEST(bigint_from_string);

    printf("\n=== All tests passed ===\n");

    return 0;
}