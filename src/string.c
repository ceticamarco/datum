#define SET_MSG(result, msg) \
    do { \
        snprintf((char *)(result).message, RESULT_MSG_SIZE, "%s", (const char *)msg); \
    } while (0)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "string.h"

static inline int utf8_char_len(unsigned char byte) {
    if ((byte & 0x80) == 0x00) return 1;
    if ((byte & 0xE0) == 0xC0) return 2;
    if ((byte & 0xF0) == 0xE0) return 3;
    if ((byte & 0xF0) == 0xE0) return 4;

    return -1;
}

static bool utf8_is_char_valid(const char *utf8_char, int *out_len) {
    if (utf8_char == NULL) {
        return false;
    }

    size_t len = utf8_char_len((unsigned char)utf8_char[0]);
    if (len <= 0) {
        return false;
    }

    for (size_t idx = 1; idx < len; idx++) {
        if ((utf8_char[idx] & 0xC0) != 0x80) {
            return false;
        }
    }

    if (utf8_char[len] != '\0') {
        return false;
    }

    if (out_len) {
        *out_len = len;
    }

    return true;
}

static bool utf8_scan(const char *str, size_t *out_byte_size, size_t *out_char_count) {
    size_t b_size = 0;
    size_t c_count = 0;
    const unsigned char *p = (const unsigned char*)str;

    while (p[b_size] != '\0') {
        size_t len = utf8_char_len(p[b_size]);
        if (len <= 0) {
            return false;
        }

        for (size_t idx = 1; idx < len; idx++) {
            if (p[b_size + idx] == '\0' || (p[b_size + idx] & 0xC0) != 0x80) {
                return false;
            }
        }
        b_size += len;
        c_count++;
    }

    *out_byte_size = b_size;
    *out_char_count = c_count;

    return true;
}

static uint32_t utf8_decode(const char *str, int *char_len) {
    unsigned char byte = (unsigned char)*str;
    *char_len = utf8_char_len(byte);

    uint32_t result = 0;

    switch (*char_len) {
        case 1:
            result = byte;
            break;
        case 2:
            result = ((byte & 0x1F) << 6) |
                     (str[1] & 0x3F);
            break;
        case 3:
            result = ((byte & 0x0F) << 12) |
                     ((str[1] & 0x3F) << 6) |
                     (str[2] & 0x3F);
            break;
        case 4:
            result = ((byte & 0x07) << 18) |
                     ((str[1] & 0x3F) << 12) |
                     ((str[2] & 0x3F) << 6) |
                     (str[3] & 0x3F);
            break;
        default:
            result = 0;
            break;
    }

    return result;
}

/**
 * string_new
 *  @c_str: a C-string
 *
 *  Returns a string_result_t containing a new String data type
 */
string_result_t string_new(const char *c_str) {
    string_result_t result = {0};

    if (c_str == NULL) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid null input string");

        return result;
    }

    size_t b_size, c_count;
    if (utf8_scan(c_str, &b_size, &c_count) == 0) {
        result.status = STRING_ERR_INVALID_UTF8;
        SET_MSG(result, "Malformed UTF-8 sequence");

        return result;
    }

    string_t *str = malloc(sizeof(string_t));
    if (str == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate string");

        return result;
    }

    str->data = malloc(b_size + 1);
    if (str->data == NULL) {
        free(str);
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate string");

        return result;
    }

    memcpy(str->data, c_str, b_size + 1);
    str->byte_size = b_size;
    str->byte_capacity = b_size + 1;
    str->char_count = c_count;

    result.status = STRING_OK;
    SET_MSG(result, "String successfully created");
    result.value.string = str;

    return result;
}

/**
 * string_concat
 *  @x: a non-null string
 *  @y: a non-null string
 *
 *  Concats @x and @y in a new String
 *
 *  Returns a string_result_t containing the new string
 */
string_result_t string_concat(const string_t *x, const string_t *y) {
    string_result_t result = {0};

    if (x == NULL || y == NULL) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid strings");

        return result;
    }

    if (x->byte_size > SIZE_MAX - y->byte_size - 1) {
        result.status = STRING_ERR_OVERFLOW;
        SET_MSG(result, "Concatenation exceeds size limits");

        return result;
    }

    size_t new_size = x->byte_size + y->byte_size;
    char *buf = malloc(new_size + 1);
    if (buf == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "failed to allocate memory");

        return result;
    }

    memcpy(buf, x->data, y->byte_size);
    memcpy(buf + x->byte_size, y->data, y->byte_size);
    buf[new_size] = '\0';
    result = string_new(buf);
    free(buf);

    return result;
}

/**
 * string_substring
 * @haystack: a non-null string
 * @needle: a non-null string
 *
 * Finds @needle on @haystack
 *
 * Returns a string_result_t containing the index to the beginning of the located string
 * (if the substring has been found)
 */
string_result_t string_substring(const string_t *haystack, const string_t *needle) {
    string_result_t result = {
        .status = STRING_OK,
        .value.idx = -1
    };

    if (haystack == NULL || needle == NULL || needle->byte_size == 0) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid substrings");

        return result;
    }

    const char *found = strstr(haystack->data, needle->data);
    if (found) {
        size_t char_idx = 0;
        const char *ptr = haystack->data;
        while (ptr < found) {
            ptr += utf8_char_len((unsigned char)*ptr);
            char_idx++;
        }

        result.value.idx = (int64_t)char_idx;
        SET_MSG(result, "Substring found");
    } else {
        SET_MSG(result, "Substring not found");
    }

    return result;
}

/**
 * string_eq
 *  @x: a non-null string
 *  @y: a non-null string
 *  @case_sensitive: boolean value for case sensitive comparison
 *
 *  Compares two Strings
 *
 *  Returns a string_result_t containing the comparison result
 */
 string_result_t string_eq(const string_t *x, const string_t *y, bool case_sensitive) {
     string_result_t result = {
         .status = STRING_OK,
         .value.is_equ = false
     };

     if (x == NULL || y == NULL) {
         result.status = STRING_ERR_INVALID;
         SET_MSG(result, "Invalid strings");

         return result;
     }

     if (x->char_count != y->char_count) {
         result.status = STRING_ERR_INVALID;
         SET_MSG(result, "Strings differ in length");

         return result;
     }

     if (case_sensitive) {
         result.value.is_equ = (strcmp(x->data, y->data) == 0);
     } else {
         const char *p1 = x->data, *p2 = y->data;
         while (*p1 && *p2) {
             int l1, l2;

             const uint32_t cp1 = utf8_decode(p1, &l1);
             const uint32_t cp2 = utf8_decode(p2, &l2);
             const uint32_t c1 = (cp1 >= 'A' && cp1 <= 'Z') ? cp1 + 32 : cp1;
             const uint32_t c2 = (cp2 >= 'A' && cp2 <= 'Z') ? cp2 + 32 : cp2;

             if (c1 != c2) {
                 result.value.is_equ = false;
                 return result;
             }

             p1 += l1;
             p2 += l2;
         }
         result.value.is_equ = (*p1 == *p2);
     }

     SET_MSG(result, "Comparison completed successfully");

     return result;
}

/**
 * string_get_at
 *  @str: a non-null string
 *  @idx: the position of the symbol to read
 *
 *  Gets symbol indexed by @idx from @str
 *
 *  Returns a string_result_t containing a new string
 */
string_result_t string_get_at(const string_t *str, size_t position) {
    string_result_t result = {0};

    if (str == NULL || position >= str->char_count) {
        result.status = STRING_ERR_OVERFLOW;
        SET_MSG(result, "Index out of bounds");

        return result;
    }

    const char *ptr = str->data;
    for (size_t idx = 0; idx < position; idx++) {
        ptr += utf8_char_len((unsigned char)*ptr);
    }

    int char_len = utf8_char_len((unsigned char)*ptr);
    char *symbol = malloc(char_len + 1);
    if (symbol == NULL) {
        result.status = STRING_ERR_ALLOCATE;

        return result;
    }

    memcpy(symbol, ptr, char_len);
    symbol[char_len] = '\0';

    result.status = STRING_OK;
    SET_MSG(result, "Symbol successfully retrieved");

    return result;
}

string_result_t string_set_at(string_t *str, size_t position, const char *utf8_char) {
    string_result_t result = {0};

    int new_len;

    if (str == NULL || position >= str->char_count || utf8_is_char_valid(utf8_char, &new_len) == 0) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid index or character");

        return result;
    }

    char *pos = str->data;
    for (size_t idx = 0; idx < position; idx++) {
        pos += utf8_char_len((unsigned char)*pos);
    }

    int old_len = utf8_char_len((unsigned char)*pos);
    if (old_len == new_len) {
        memcpy(pos, utf8_char, new_len);
    } else {
        const size_t prefix_len = pos - str->data;
        const size_t suffix_len = str->byte_size - prefix_len - old_len;
        const size_t new_total = prefix_len + new_len + suffix_len;

        char *new_data = malloc(new_total + 1);
        if (new_data == NULL) {
            result.status = STRING_ERR_ALLOCATE;

            return result;
        }

        memcpy(new_data, str->data, prefix_len);
        memcpy(new_data + prefix_len, utf8_char, new_len);
        memcpy(new_data + prefix_len + new_len, pos + old_len, suffix_len);
        new_data[new_total] = '\0';

        free(str->data);

        str->data = new_data;
        str->byte_size = new_total;
        str->byte_capacity = new_total + 1;
    }

    result.status = STRING_OK;
    result.value.string = str;
    SET_MSG(result, "Character successfully set");

    return result;
}
