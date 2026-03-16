/*
 * Bigint operations example.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/bigint.h"

int main(void) {
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

    bigint_t *a = bigint_from_string(large_x).value.number;
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

