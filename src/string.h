#ifndef STRING_H
#define STRING_H

#define RESULT_MSG_SIZE 64

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    STRING_OK = 0x0,
    STRING_ERR_ALLOCATE,
    STRING_ERR_INVALID,
    STRING_ERR_INVALID_UTF8,
    STRING_ERR_OVERFLOW
} string_status_t;

typedef struct {
    char *data;
    size_t byte_size; // Size in bytes excluding NULL terminator
    size_t byte_capacity; // total allocated memory
    size_t char_count; // Number of symbols
} string_t;

typedef struct {
    string_status_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        string_t *string; // For new, reverse, trim
        char *c_str; // For get_at
        int64_t idx; // For substring search
        bool is_equ; // For comparison
        struct { // For split
            string_t **strings;
            size_t count;
        } split;
    } value;
} string_result_t;

// Public APIs
string_result_t string_new(const char *c_str);
string_result_t string_concat(const string_t *x, const string_t *y);
string_result_t string_substring(const string_t *haystack, const string_t *needle);
string_result_t string_eq(const string_t *x, const string_t *y, bool case_sensitive);
string_result_t string_get_at(const string_t *str, size_t idx);
string_result_t string_set_at(string_t *str, size_t idx);
string_result_t string_to_lower(const string_t *str);
string_result_t string_to_upper(const string_t *str);
string_result_t string_reverse(const string_t *str);
string_result_t string_trim(const string_t *str);
string_result_t string_split(const string_t *str, const char *delim);
string_result_t string_destroy(string_t *str);
string_result_t string_split_destroy(string_t **split, size_t counT);

#endif
