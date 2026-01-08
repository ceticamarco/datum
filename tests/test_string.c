/*
 * Unit tests for String data type
*/

#define TEST(NAME) do { \
    printf("Running test_%s...", #NAME); \
    test_##NAME(); \
    printf(" PASSED\n"); \
} while(0)

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../src/string.h"

// Test string creation
void test_string_new(void) {
    string_result_t res = string_new("hello");

    assert(res.status == STRING_OK);
    assert(res.value.string != NULL);
    assert(strcmp(res.value.string->data, "hello") == 0);
    assert(string_len(res.value.string) == 5);
    assert(res.value.string->byte_size == 5);

    string_destroy(res.value.string);
}

// Test empty string
void test_string_new_empty(void) {
    string_result_t res = string_new("");

    assert(res.status == STRING_OK);
    assert(string_len(res.value.string) == 0);
    assert(res.value.string->byte_size == 0);
    assert(res.value.string->data[0] == '\0');

    string_destroy(res.value.string);
}

// Test string concatenation
void test_string_concat(void) {
    string_t *str1 = string_new("Foo").value.string;
    string_t *str2 = string_new(" Bar").value.string;

    string_result_t res = string_concat(str1, str2);
    assert(res.status == STRING_OK);
    assert(strcmp(res.value.string->data, "Foo Bar") == 0);
    assert(string_len(res.value.string) == 7);

    string_destroy(str1);
    string_destroy(str2);
    string_destroy(res.value.string);
}

// Test case-insensitive and sensitive comparison
void test_string_eq(void) {
    string_t *str1 = string_new("Foo").value.string;
    string_t *str2 = string_new("foo").value.string;

    // Case sensitive comparison should be false
    assert(string_eq(str1, str2, true).value.is_equ == false);
    // Case insensitive comparison should be true
    assert(string_eq(str1, str2, false).value.is_equ == true);

    string_destroy(str1);
    string_destroy(str2);
}

// Test string reverse using UTF-8 symbols
void test_string_reverse_utf8(void) {
    string_t *str = string_new("A🌍Z").value.string;

    string_result_t res = string_reverse(str);
    
    assert(res.status == STRING_OK);
    assert(string_len(res.value.string) == 3);
    assert(strcmp(res.value.string->data, "Z🌍A") == 0);
    assert(string_len(res.value.string) == 3);

    string_destroy(str);
    string_destroy(res.value.string);
}

// Test mutation of UTF-8 symbol
void test_string_set_at(void) {
    string_t *str = string_new("ABC").value.string;

    // Replace 'B' with emoji
    string_result_t res = string_set_at(str, 1, "😆");
    assert(res.status == STRING_OK);
    assert(strcmp(str->data, "A😆C") == 0);
    assert(string_len(str) == 3);
    assert(str->byte_size == 6); // that is: A (1B) + emoji (4B) + C (1B)

    string_destroy(str);
}

// Test mutation with overflow
void test_string_set_at_overflow(void) {
    string_t *str = string_new("ABC").value.string;

    string_result_t res = string_set_at(str, 10, "a");
    assert(res.status == STRING_ERR_OVERFLOW);

    string_destroy(str);
}

// Test whitespace trimming
void test_string_trim(void) {
    string_t *str = string_new("   \t   Foo Bar \n    ").value.string;

    string_result_t res = string_trim(str);
    assert(res.status == STRING_OK);
    assert(strcmp(res.value.string->data, "Foo Bar") == 0);

    string_destroy(str);
    string_destroy(res.value.string);
}

// Test string splitting into an array
void test_string_split(void) {
    string_t *str = string_new("Red,Green,Blue").value.string;

    string_result_t res = string_split(str, ",");
    assert(res.status == STRING_OK);
    assert(res.value.split.count == 3);

    assert(strcmp(res.value.split.strings[0]->data, "Red") == 0);
    assert(strcmp(res.value.split.strings[1]->data, "Green") == 0);
    assert(strcmp(res.value.split.strings[2]->data, "Blue") == 0);

    string_split_destroy(res.value.split.strings, res.value.split.count);
    string_destroy(str);
}

int main(void) {
    printf("=== Running Vector unit tests ===\n\n");

    TEST(string_new);
    TEST(string_new_empty);
    TEST(string_concat);
    TEST(string_eq);
    TEST(string_reverse_utf8);
    TEST(string_set_at);
    TEST(string_set_at_overflow);
    TEST(string_trim);
    TEST(string_split);

    printf("\n=== All tests passed! ===\n");
    return 0;
}
