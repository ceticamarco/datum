/*
 * Advanced string manipulation example.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/string.h"

int main(void) {
    // Create a string for manipulation
    string_result_t res = string_new("Hello, World! 😜");
    if (res.status != STRING_OK) {
        printf("Error: %s\n", res.message);
        return 1;
    }

    string_t *str = res.value.string;
    printf("Original string: \"%s\"\n\n", str->data);

    // Uppercase string
    string_result_t res_upper = string_to_upper(str);
    if (res_upper.status != STRING_OK) {
        printf("Error: %s\n", res_upper.message);
        return 1;
    }
    printf("Uppercase: \"%s\"\n", res_upper.value.string->data);
    string_destroy(res_upper.value.string);

    // Lowercase string
    string_result_t res_lower = string_to_lower(str);
    if (res_lower.status != STRING_OK) {
        printf("Error: %s\n", res_lower.message);
        return 1;
    }
    printf("Lowercase: \"%s\"\n\n", res_lower.value.string->data);
    string_destroy(res_lower.value.string);

    // Reverse string
    string_result_t res_rev = string_reverse(str);
    if (res_rev.status != STRING_OK) {
        printf("Error: %s\n", res_rev.message);
        return 1;
    }
    printf("Reversed: \"%s\"\n\n", res_rev.value.string->data);
    string_destroy(res_rev.value.string);

    // Change first character of the string
    string_result_t res_set = string_set_at(str, 0, "J");
    if (res_set.status != STRING_OK) {
        printf("Error: %s\n", res_set.message);
        return 1;
    }
    printf("Updated string: \"%s\"\n\n", res_set.value.string->data);
    string_destroy(res_set.value.string);

    // Get character from string (the emoji)
    string_result_t res_get = string_get_at(str, 14);
    if (res_get.status != STRING_OK) {
        printf("Error: %s\n", res_get.message);
        return 1;
    }
    printf("Extracted symbol: \"%s\"\n", res_get.value.symbol);
    free(res_get.value.symbol);

    // Trim string
    string_t *to_trim = string_new("    foo    ").value.string;
    string_result_t res_trim = string_trim(to_trim);
    if (res_trim.status != STRING_OK) {
        printf("Error: %s\n", res_trim.message);
        return 1;
    }

    printf("Trimmed string: \"%s\"\n\n", res_trim.value.string->data);
    string_destroy(to_trim);
    string_destroy(res_trim.value.string);

    // Split string
    string_t *to_split = string_new("foo/bar/biz").value.string;
    string_result_t res_split = string_split(to_split, "/");
    if (res_split.status != STRING_OK) {
        printf("Error: %s\n", res_split.message);
        return 1;
    }

    const size_t count = res_split.value.split.count;
    string_t **strings = res_split.value.split.strings;

    printf("Original string: \"%s\"\nSplit string: ", to_split->data);
    for (size_t idx = 0; idx < count; idx++) {
        printf("\"%s\" ", strings[idx]->data);
    }

    printf("\n");

    string_split_destroy(strings, count);
    string_destroy(to_split);

    string_destroy(str);

    return 0;
}

