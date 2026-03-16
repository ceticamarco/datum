/*
 * Basic string operations example.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/string.h"

int main(void) {
    // Create a new string
    string_result_t res = string_new("Hello, ");
    if (res.status != STRING_OK) {
        printf("Error: %s\n", res.message);
        return 1;
    }

    string_t *str1 = res.value.string;
    printf("Created string: \"%s\"\n", str1->data);
    printf("Character count: %zu (%zu actual bytes)\n", string_size(str1), str1->byte_size);

    string_result_t res_clone = string_clone(str1);
    if (res_clone.status != STRING_OK) {
        printf("Error: %s\n", res.message);
        return 1;
    }

    string_t *cloned = res_clone.value.string;
    printf("Cloned string: \"%s\"\n\n", cloned->data);
    string_destroy(cloned);

    // Concatenation of strings
    string_result_t res_suffix = string_new("World! 😜");
    if (res_suffix.status != STRING_OK) {
        printf("Error: %s\n", res.message);
        return 1;
    }

    string_t *suffix = res_suffix.value.string;
    printf("Created another string: \"%s\"\n", suffix->data);
    printf("Character count: %zu (%zu actual bytes)\n\n", string_size(suffix), suffix->byte_size);

    string_result_t res_cat = string_concat(str1, suffix);
    if (res_cat.status != STRING_OK) {
        printf("Error: %s\n", res_cat.message);
        return 1;
    }
    string_destroy(suffix);

    string_t *concat_str = res_cat.value.string;
    printf("Concatenation result: \"%s\"\n\n", concat_str->data);

    // String contains
    string_t *haystack = string_new("The quick brown fox jumps over the lazy dog.").value.string;
    string_t *needle = string_new("brown fox").value.string;

    string_result_t res_contains = string_contains(haystack, needle);
    if (res_contains.status != STRING_OK) {
        printf("Error: %s\n", res_contains.message);
        return 1;
    }

    if (res_contains.value.idx != -1) {
        printf("Substring found. Starting at index %ld\n\n", res_contains.value.idx);
    }

    string_destroy(haystack);
    string_destroy(needle);

    // String slicing
    string_result_t res_slice = string_slice(concat_str, 7, 14);
    if (res_slice.status != STRING_OK) {
        printf("Error: %s\n", res_slice.message);
        return 1;
    }

    printf("Slice of string: \"%s\"\n\n", res_slice.value.string->data);
    string_destroy(res_slice.value.string);

    // String equality
    string_t *compare = string_new("hello, World! 😜").value.string;
    string_result_t res_eq = string_eq(concat_str, compare, true);
    if (res_eq.value.is_equ) {
        printf("The two strings are equal\n\n");
    } else {
        printf("The two strings are not equal\n\n");
    }

    string_destroy(compare);
    string_destroy(concat_str);
    string_destroy(str1);

    return 0;
}

