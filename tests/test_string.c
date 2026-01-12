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
#include <stdlib.h>

#include "../src/string.h"

// Test string creation
void test_string_new(void) {
    string_result_t res = string_new("hello");

    assert(res.status == STRING_OK);
    assert(res.value.string != NULL);
    assert(strcmp(res.value.string->data, "hello") == 0);
    assert(string_size(res.value.string) == 5);
    assert(res.value.string->byte_size == 5);

    string_destroy(res.value.string);
}

// Test empty string
void test_string_new_empty(void) {
    string_result_t res = string_new("");

    assert(res.status == STRING_OK);
    assert(string_size(res.value.string) == 0);
    assert(res.value.string->byte_size == 0);
    assert(res.value.string->data[0] == '\0');

    string_destroy(res.value.string);
}

// Test cloning an existing string
void test_string_clone(void) {
    string_t *original = string_new("Original").value.string;
    string_result_t res = string_clone(original);

    assert(res.status == STRING_OK);
    assert(res.value.string != original); // Different memory address
    assert(strcmp(res.value.string->data, original->data) == 0);
    assert(res.value.string->byte_size == original->byte_size);

    string_destroy(original);
    string_destroy(res.value.string);
}

// Test string concatenation
void test_string_concat(void) {
    string_t *str1 = string_new("Foo").value.string;
    string_t *str2 = string_new(" Bar").value.string;

    string_result_t res = string_concat(str1, str2);
    assert(res.status == STRING_OK);
    assert(strcmp(res.value.string->data, "Foo Bar") == 0);
    assert(string_size(res.value.string) == 7);

    string_destroy(str1);
    string_destroy(str2);
    string_destroy(res.value.string);
}

// Test if string contains a substring
void test_string_contains(void) {
    string_t *haystack = string_new("Hello 🌍 World").value.string;
    string_t *needle_ascii = string_new("World").value.string;
    string_t *needle_utf8 = string_new("🌍").value.string;
    string_t *needle_none = string_new("not found").value.string;

    // World starts at symbol 8
    string_result_t res1 = string_contains(haystack, needle_ascii);
    assert(res1.status == STRING_OK);
    assert(res1.value.idx == 8);

    // 🌍 is at position 6
    string_result_t res2 = string_contains(haystack, needle_utf8);
    assert(res2.status == STRING_OK);
    assert(res2.value.idx == 6);

    // Not found should return -1
    string_result_t res3 = string_contains(haystack, needle_none);
    assert(res3.status == STRING_OK);
    assert(res3.value.idx == -1);

    string_destroy(haystack);
    string_destroy(needle_ascii);
    string_destroy(needle_utf8);
    string_destroy(needle_none);
}

// Test string slicing
void test_string_slice(void) {
    // ASCII slice
    string_t *str1 = string_new("foobar").value.string;
    string_result_t res1 = string_slice(str1, 2, 4);

    assert(res1.status == STRING_OK);
    assert(strcmp(res1.value.string->data, "oba") == 0);
    assert(res1.value.string->char_count == 3);

    // UTF-8 slice
    string_t *str2 = string_new("AB😆🌍").value.string;
    string_result_t res2 = string_slice(str2, 2, 2);

    assert(res2.status == STRING_OK);
    assert(strcmp(res2.value.string->data, "😆") == 0);
    assert(res2.value.string->byte_size == 4); // emoji = 4 bytes

    // UTF-8 + ASCII slice
    string_result_t res3 = string_slice(str2, 0, 2);
    assert(res3.status == STRING_OK);
    assert(strcmp(res3.value.string->data, "AB😆") == 0);

    // Invalid bounds
    string_result_t res4 = string_slice(str1, 5, 2);
    assert(res4.status == STRING_ERR_OVERFLOW);

    res4 = string_slice(str1, 1, 50);
    assert(res4.status == STRING_ERR_OVERFLOW);

    string_destroy(str1);
    string_destroy(str2);
    string_destroy(res1.value.string);
    string_destroy(res2.value.string);
    string_destroy(res3.value.string);
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
    assert(string_size(res.value.string) == 3);
    assert(strcmp(res.value.string->data, "Z🌍A") == 0);
    assert(string_size(res.value.string) == 3);

    string_destroy(str);
    string_destroy(res.value.string);
}

// Test string get_at
void test_string_get_at(void) {
    string_t *str = string_new("AB😆🌍").value.string;

    // 😆 is at index 2
    string_result_t res1 = string_get_at(str, 2);
    assert(res1.status == STRING_OK);
    assert(strcmp((char*)res1.value.symbol, "😆") == 0);
    free(res1.value.symbol);
    
    // 🌍 is at index 3
    string_result_t res2 = string_get_at(str, 3);
    assert(res2.status == STRING_OK);
    assert(strcmp((char*)res2.value.symbol, "🌍") == 0);
    free(res2.value.symbol);

    string_destroy(str);
}

// Test string get_at with invalid index
void test_string_get_at_overflow(void) {
    string_t *str = string_new("ABC").value.string;

    string_result_t res = string_get_at(str, 50);
    assert(res.status == STRING_ERR_OVERFLOW);

    string_destroy(str);
}

// Test mutation of UTF-8 symbol
void test_string_set_at(void) {
    string_t *str = string_new("ABC").value.string;

    // Replace 'B' with emoji
    string_result_t res = string_set_at(str, 1, "😆");
    string_t *altered = res.value.string;
    
    assert(res.status == STRING_OK);
    assert(strcmp(altered->data, "A😆C") == 0);
    assert(string_size(altered) == 3);
    assert(altered->byte_size == 6); // that is: A (1B) + emoji (4B) + C (1B)

    string_destroy(str);
    string_destroy(altered);
}

// Test mutation of invalid UTF-8 symbol
void test_string_set_at_invalid_utf8(void) {
    string_t *str = string_new("ABC").value.string;

    const char * const invalid_sym1 = "\xFF";
    const char * const invalid_sym2 = "\x80";
    
    string_result_t res1 = string_set_at(str, 1, invalid_sym1);
    assert(res1.status == STRING_ERR_INVALID_UTF8);

    string_result_t res2 = string_set_at(str, 1, invalid_sym2);
    assert(res2.status == STRING_ERR_INVALID_UTF8);

    string_destroy(str);
}

// Test mutation with overflow
void test_string_set_at_overflow(void) {
    string_t *str = string_new("ABC").value.string;

    string_result_t res = string_set_at(str, 10, "a");
    assert(res.status == STRING_ERR_OVERFLOW);

    string_destroy(str);
}

// Test string to lowercase
void test_string_to_lower(void) {
    string_t *str = string_new("AbC").value.string;
    string_result_t res = string_to_lower(str);

    assert(res.status == STRING_OK);
    assert(strcmp(res.value.string->data, "abc") == 0);

    string_destroy(str);
    string_destroy(res.value.string);
}

// Test string to uppercase
void test_string_to_upper(void) {
    string_t *str = string_new("aBc").value.string;
    string_result_t res = string_to_upper(str);

    assert(res.status == STRING_OK);
    assert(strcmp(res.value.string->data, "ABC") == 0);

    string_destroy(str);
    string_destroy(res.value.string);
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

    const size_t count = res.value.split.count;
    string_t **strings = res.value.split.strings;

    const char *expected[] = { "Red", "Green", "Blue" };
    for (size_t idx = 0; idx < count; idx++) {
        assert(strcmp(strings[idx]->data, expected[idx]) == 0);
    }
  
    string_split_destroy(strings, count);
    string_destroy(str);
}

// Test string destroy
void test_string_destroy(void) {
    string_t *str = string_new("delete me").value.string;

    string_result_t res = string_destroy(str);
    assert(res.status == STRING_OK);

    string_result_t res_null = string_destroy(NULL);
    assert(res_null.status == STRING_ERR_INVALID);
}

int main(void) {
    printf("=== Running String unit tests ===\n\n");

    TEST(string_new);
    TEST(string_new_empty);
    TEST(string_clone);
    TEST(string_concat);
    TEST(string_contains);
    TEST(string_slice);
    TEST(string_eq);
    TEST(string_reverse_utf8);
    TEST(string_get_at);
    TEST(string_get_at_overflow);
    TEST(string_set_at);
    TEST(string_set_at_overflow);
    TEST(string_set_at_invalid_utf8);
    TEST(string_to_lower);
    TEST(string_to_upper);
    TEST(string_trim);
    TEST(string_split);
    TEST(string_destroy);

    printf("\n=== All tests passed! ===\n");
    return 0;
}
