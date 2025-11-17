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
    bigint_result_t exp_num_res = bigint_from_string(expected);
    assert(exp_num_res.status == BIGINT_OK);
    bigint_t *exp_num = exp_num_res.value.number;

    bigint_result_t cmp_res = bigint_compare(number, exp_num);
    assert(cmp_res.status == BIGINT_OK);
    
    const int8_t cmp = cmp_res.value.compare_status;
    assert(cmp == 0);

    bigint_destroy(exp_num);
}

// Test creating big integers from int
void test_bigint_from_int(void) {
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
void test_bigint_from_string(void) {
    bigint_result_t res = bigint_from_string("00000123");

    assert(res.status == BIGINT_OK);
    bigint_eq(res.value.number, "123");
    bigint_destroy(res.value.number);

    res = bigint_from_string("-00000456789");
    assert(res.status == BIGINT_OK);
    bigint_eq(res.value.number, "-456789");
    bigint_destroy(res.value.number);
}

// Test sum between big integers
void test_bigint_add(void) {
    bigint_result_t x = bigint_from_int(123);
    bigint_result_t y = bigint_from_int(456);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t sum = bigint_add(x.value.number, y.value.number);
    assert(sum.status == BIGINT_OK);
    bigint_eq(sum.value.number, "579");

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
    bigint_destroy(sum.value.number);
}

// Test difference between big numbers
void test_bigint_sub(void) {
    bigint_result_t x = bigint_from_int(456);
    bigint_result_t y = bigint_from_int(123);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t diff = bigint_sub(x.value.number, y.value.number);
    assert(diff.status == BIGINT_OK);
    bigint_eq(diff.value.number, "333");

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
    bigint_destroy(diff.value.number);
}

// Test difference between big numbers with negative result
void test_bigint_sub_neg(void) {
    bigint_result_t x = bigint_from_int(123);
    bigint_result_t y = bigint_from_int(456);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t diff = bigint_sub(x.value.number, y.value.number);
    assert(diff.status == BIGINT_OK);
    bigint_eq(diff.value.number, "-333");

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
    bigint_destroy(diff.value.number);
}

// Test difference between mixed big numbers
void test_bigint_sub_mixed(void) {
    bigint_result_t x = bigint_from_int(456);
    bigint_result_t y = bigint_from_int(-123);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t diff = bigint_sub(x.value.number, y.value.number);
    assert(diff.status == BIGINT_OK);
    bigint_eq(diff.value.number, "579");

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
    bigint_destroy(diff.value.number);
}

// Test product between big numbers
void test_bigint_prod(void) {
    bigint_result_t x = bigint_from_int(1234);
    bigint_result_t y = bigint_from_int(56789);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t prod = bigint_prod(x.value.number, y.value.number);
    assert(prod.status == BIGINT_OK);
    bigint_eq(prod.value.number, "70077626");

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
    bigint_destroy(prod.value.number);
}

// Test product between mixed negative big numbers
void test_bigint_prod_mixed(void) {
    bigint_result_t x = bigint_from_int(-1234);
    bigint_result_t y = bigint_from_int(56789);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t prod = bigint_prod(x.value.number, y.value.number);
    assert(prod.status == BIGINT_OK);
    bigint_eq(prod.value.number, "-70077626");

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
    bigint_destroy(prod.value.number);
}

// Test product between negative big numbers
void test_bigint_prod_neg(void) {
    bigint_result_t x = bigint_from_int(-1234);
    bigint_result_t y = bigint_from_int(-56789);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t prod = bigint_prod(x.value.number, y.value.number);
    assert(prod.status == BIGINT_OK);
    bigint_eq(prod.value.number, "70077626");

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
    bigint_destroy(prod.value.number);
}

// Test division between big numbers
void test_bigint_div(void) {
    bigint_result_t x = bigint_from_int(100);
    bigint_result_t y = bigint_from_int(2);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t div = bigint_divmod(x.value.number, y.value.number);
    assert(div.status == BIGINT_OK);

    bigint_t* const quotient = div.value.division.quotient;
    bigint_t* const remainder = div.value.division.remainder;

    bigint_eq(quotient, "50");
    bigint_eq(remainder, "0");

    bigint_destroy(quotient);
    bigint_destroy(remainder);

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
}

// Test division between big numbers with negative dividend
// This library follows C-style divison such that sign(remainder) = sign(dividend)
void test_bigint_div_dividend(void) {
    bigint_result_t x = bigint_from_int(-100);
    bigint_result_t y = bigint_from_int(3);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t div = bigint_divmod(x.value.number, y.value.number);
    assert(div.status == BIGINT_OK);

    bigint_t* const quotient = div.value.division.quotient;
    bigint_t* const remainder = div.value.division.remainder;

    bigint_eq(quotient, "-33");
    bigint_eq(remainder, "-1");

    bigint_destroy(quotient);
    bigint_destroy(remainder);

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
}

// Test division between big numbers with negative divisor
// This library follows C-style divison such that sign(remainder) = sign(dividend)
void test_bigint_div_divisor(void) {
    bigint_result_t x = bigint_from_int(13);
    bigint_result_t y = bigint_from_int(-4);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t div = bigint_divmod(x.value.number, y.value.number);
    assert(div.status == BIGINT_OK);

    bigint_t* const quotient = div.value.division.quotient;
    bigint_t* const remainder = div.value.division.remainder;

    bigint_eq(quotient, "-3");
    bigint_eq(remainder, "1");

    bigint_destroy(quotient);
    bigint_destroy(remainder);

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
}

// Test division between big numbers with negative numbers
// This library follows C-style divison such that sign(remainder) = sign(dividend)
void test_bigint_div_neg(void) {
    bigint_result_t x = bigint_from_int(-100);
    bigint_result_t y = bigint_from_int(-3);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t div = bigint_divmod(x.value.number, y.value.number);
    assert(div.status == BIGINT_OK);

    bigint_t* const quotient = div.value.division.quotient;
    bigint_t* const remainder = div.value.division.remainder;

    bigint_eq(quotient, "33");
    bigint_eq(remainder, "-1");

    bigint_destroy(quotient);
    bigint_destroy(remainder);

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
}

// Test division by zero
void test_bigint_div_by_zero(void) {
    bigint_result_t x = bigint_from_int(-100);
    bigint_result_t y = bigint_from_int(0);

    assert(x.status == BIGINT_OK && y.status == BIGINT_OK);

    bigint_result_t div = bigint_divmod(x.value.number, y.value.number);
    assert(div.status == BIGINT_ERR_DIV_BY_ZERO);

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
}

// Test cloning of big numbers
void test_bigint_clone(void) {
    bigint_result_t x = bigint_from_string("0010101010");

    assert(x.status == BIGINT_OK);

    bigint_result_t cloned = bigint_clone(x.value.number);
    assert(cloned.status == BIGINT_OK);

    bigint_eq(cloned.value.number, "10101010");

    bigint_destroy(x.value.number);
    bigint_destroy(cloned.value.number);
}

// Test comparison between equal numbers
void test_bigint_compare_eq(void) {
    bigint_result_t x = bigint_from_int(123);
    bigint_result_t y = bigint_from_int(123);

    assert(x.status == BIGINT_OK);
    assert(y.status == BIGINT_OK);

    bigint_result_t cmp_res = bigint_compare(x.value.number, y.value.number);
    assert(cmp_res.status == BIGINT_OK);
    
    const int8_t cmp = cmp_res.value.compare_status;
    assert(cmp == 0);

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
}

// Test comparison between numbers (less than)
void test_bigint_compare_lt(void) {
    bigint_result_t x = bigint_from_int(-123);
    bigint_result_t y = bigint_from_int(0);

    assert(x.status == BIGINT_OK);
    assert(y.status == BIGINT_OK);

    bigint_result_t cmp_res = bigint_compare(x.value.number, y.value.number);
    assert(cmp_res.status == BIGINT_OK);
    
    const int8_t cmp = cmp_res.value.compare_status;
    assert(cmp == -1);

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
}

// Test comparison between numbers (greater than)
void test_bigint_compare_gt(void) {
    bigint_result_t x = bigint_from_int(123);
    bigint_result_t y = bigint_from_int(-5);

    assert(x.status == BIGINT_OK);
    assert(y.status == BIGINT_OK);

    bigint_result_t cmp_res = bigint_compare(x.value.number, y.value.number);
    assert(cmp_res.status == BIGINT_OK);
    
    const int8_t cmp = cmp_res.value.compare_status;
    assert(cmp == 1);

    bigint_destroy(x.value.number);
    bigint_destroy(y.value.number);
}


int main(void) {
    printf("=== Running BigInt unit tests ===\n\n");

    TEST(bigint_from_int);
    TEST(bigint_from_string);
    TEST(bigint_add);
    TEST(bigint_sub);
    TEST(bigint_sub_neg);
    TEST(bigint_sub_mixed);
    TEST(bigint_prod);
    TEST(bigint_prod_mixed);
    TEST(bigint_prod_neg);
    TEST(bigint_div);
    TEST(bigint_div_dividend);
    TEST(bigint_div_divisor);
    TEST(bigint_div_neg);
    TEST(bigint_div_by_zero);
    TEST(bigint_clone);
    TEST(bigint_compare_eq);
    TEST(bigint_compare_lt);
    TEST(bigint_compare_gt);

    printf("\n=== All tests passed ===\n");

    return 0;
}
