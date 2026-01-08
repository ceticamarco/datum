#define SET_MSG(result, msg) \
    do { \
        snprintf((char *)(result).message, RESULT_MSG_SIZE, "%s", (const char *)msg); \
    } while (0)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "string.h"

// Check if a character is a space
static inline bool is_space(unsigned char c) {
    return (c == ' ' || c == '\t' ||
            c == '\n' || c == '\r' ||
            c == '\f' || c == '\v');
}

// Get byte length of a UTF-8 character/symbol
static inline int utf8_char_len(unsigned char byte) {
    if ((byte & 0x80) == 0x00) return 1;
    if ((byte & 0xE0) == 0xC0) return 2;
    if ((byte & 0xF0) == 0xE0) return 3;
    if ((byte & 0xF8) == 0xF0) return 4;

    return -1;
}

// Validate an UTF-8 symbol
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

// Validate an UTF-8 symbol and measure byte length and character count in one pass
static bool utf8_scan(const char *str, size_t *out_byte_size, size_t *out_char_count) {
    size_t b_size = 0;
    size_t c_count = 0;
    const unsigned char *p = (const unsigned char *)str;

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

// Decode an UTF-8 symbol to a codepoint
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

// Encode a codepoint to an UTF-8 symbol
static int utf8_encode(uint32_t codepoint, char *out) {
    if (codepoint <= 0x7F) {
        out[0] = (char)codepoint;
        return 1;
    }

    if (codepoint <= 0x7FF) {
        out[0] = (char)(0xC0 | (codepoint >> 6));
        out[1] = (char)(0x80 | (codepoint & 0x3F));

        return 2;
    }

    if (codepoint <= 0xFFFF) {
        out[0] = (char)(0xE0 | (codepoint >> 12));
        out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out[2] = (char)(0x80 | (codepoint & 0x3F));

        return 3;
    }

    if (codepoint <= 0x10FFFF) {
        out[0] = (char)(0xF0 | (codepoint >> 18));
        out[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out[3] = (char)(0x80 | (codepoint & 0x3F));

        return 4;
    }

    return 0;
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
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    str->data = malloc(b_size + 1);
    if (str->data == NULL) {
        free(str);
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

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
 * string_clone
 *  @str: a non-null string
 *
 *  Deep copies @str
 *
 *  Returns a string_result_t containing the copied string
 */
string_result_t string_clone(const string_t *str) {
    string_result_t result = {0};

    if (str == NULL) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid string");

        return result;
    }

    string_t *str_copy = malloc(sizeof(string_t));
    if (str_copy == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    str_copy->data = malloc(str->byte_size + 1);
    if (str_copy->data == NULL) {
        free(str_copy);
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    memcpy(str_copy->data, str->data, str->byte_size + 1);
    str_copy->byte_size = str->byte_size + 1;
    str_copy->byte_capacity = str->byte_size + 1;
    str_copy->char_count = str->char_count;

    result.status = STRING_OK;
    result.value.string = str_copy;
    SET_MSG(result, "String successfully copied");

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
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    memcpy(buf, x->data, x->byte_size);
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

             const uint32_t codepoint1 = utf8_decode(p1, &l1);
             const uint32_t codepoint2 = utf8_decode(p2, &l2);
             const uint32_t c1 = (codepoint1 >= 'A' && codepoint1 <= 'Z') ? codepoint1 + 32 : codepoint1;
             const uint32_t c2 = (codepoint2 >= 'A' && codepoint2 <= 'Z') ? codepoint2 + 32 : codepoint2;

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
 *  @position: the position of the symbol to read
 *
 *  Gets symbol indexed by @position from @str
 *
 *  Returns a string_result_t containing the symbol as a C string
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
    char *utf8_char = malloc(char_len + 1);
    if (utf8_char == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    memcpy(utf8_char, ptr, char_len);
    utf8_char[char_len] = '\0';

    result.value.symbol = utf8_char;
    result.status = STRING_OK;
    SET_MSG(result, "Symbol successfully retrieved");

    return result;
}

/**
 * string_set_at
 *  @str: a non-null string
 *  @position: the position to write into
 *  @utf8_char: an UTF8 symbol
 *
 *  Writes @utf8_char into @str at index @position
 *
 *  Returns a string_result_t data type
 */
string_result_t string_set_at(string_t *str, size_t position, const char *utf8_char) {
    string_result_t result = {0};

    int new_len;

    if (str == NULL || utf8_is_char_valid(utf8_char, &new_len) == 0) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid index or character");

        return result;
    }

    if (position >= str->char_count) {
        result.status = STRING_ERR_OVERFLOW;
        SET_MSG(result, "Index out of bounds");

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
            SET_MSG(result, "Cannot allocate memory");

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
    SET_MSG(result, "Character successfully set");

    return result;
}

/**
 * string_to_lower
 *  @str: a non-null string
 *
 *  Converts a String to lowercase
 *
 *  Returns a string_result_t containing a new string
 */
string_result_t string_to_lower(const string_t *str) {
    string_result_t result = {0};

    if (str == NULL) {
        result.status = STRING_ERR_INVALID;
         SET_MSG(result, "Invalid string");

        return result;
    }

    char *buf = malloc(str->byte_capacity);
    if (buf == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    const char *src = str->data;
    char *dst = buf;

    while (*src) {
        int len;
        uint32_t codepoint = utf8_decode(src, &len);
        uint32_t lower = (codepoint >= 'A' && codepoint <= 'Z') ? codepoint + 32 : codepoint;
        dst += utf8_encode(lower, dst);
        src += len;
    }
    *dst = '\0';
    result = string_new(buf);
    free(buf);

    result.status = STRING_OK;
    SET_MSG(result, "String successfully converted to lowercase");

    return result;
}

/**
 * string_to_upper
 *  @str: a non-null string
 *
 *  Converts a String to uppercase
 *
 *  Returns a string_result_t containing a new string
 */
string_result_t string_to_upper(const string_t *str) {
    string_result_t result = {0};

    if (str == NULL) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid string");

        return result;
    }

    char *buf = malloc(str->byte_capacity);
    if (buf == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    const char *src = str->data;
    char *dst = buf;
    while (*src) {
        int len;
        uint32_t codepoint = utf8_decode(src, &len);
        uint32_t upper = (codepoint >= 'a' && codepoint <= 'z') ? codepoint - 32 : codepoint;
        dst += utf8_encode(upper, dst);
        src += len;
    }
    *dst = '\0';
    result = string_new(buf);
    free(buf);

    result.status = STRING_OK;
    SET_MSG(result, "String successfully converted to uppercase");

    return result;
}

/**
 * string_reverse
 *  @str: a non-null string
 *
 *  Reverses @str
 *
 *  Returns a new string_result_t containing the reversed string
 */
string_result_t string_reverse(const string_t *str) {
    string_result_t result = {0};

    if (str == NULL) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid string");

        return result;
    }

    char *buf = malloc(str->byte_capacity);
    if (buf == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    const char **pos = malloc(str->char_count * sizeof(char *));
    if (pos == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    const char *ptr = str->data;
    for (size_t idx = 0; idx < str->char_count; idx++) {
        pos[idx] = ptr;
        ptr += utf8_char_len((unsigned char)*ptr);
    }

    char *dst = buf;
    for (int64_t idx = (int64_t)str->char_count - 1; idx >= 0; idx--) {
        int len = utf8_char_len((unsigned char)*pos[idx]);
        memcpy(dst, pos[idx], len);
        dst += len;
    }

    *dst = '\0';
    free(pos);
    result = string_new(buf);
    free(buf);

    SET_MSG(result, "String successfully reversed");

    return result;
}

/**
 * string_trim
 *  @str: a non-null string
 *
 *  Trims whitespace from @str
 *
 *  Returns a string_result_t containing the trimmed string
 */
string_result_t string_trim(const string_t *str) {
    string_result_t result = {0};

    if (str == NULL) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid string");

        return result;
    }

    const char *start = str->data;
    while (*start && is_space((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        return string_new("");
    }

    const char *end = str->data + str->byte_size - 1;
    while (end > start && is_space((unsigned char)*end)) {
        end--;
    }

    const size_t len = (end - start) + 1;
    char *trimmed = malloc(len + 1);
    if (trimmed == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    memcpy(trimmed, start, len);
    trimmed[len] = '\0';
    result = string_new(trimmed);
    free(trimmed);

    result.status = STRING_OK;
    SET_MSG(result, "String successfully trimmed");

    return result;
}

/**
 * string_split
 *  @str: a non-null string
 *  @delim: delimiter string
 *
 *  Splits @str by @delim
 *
 *  Returns a string_result_t containing an array of String pointers
 */
string_result_t string_split(const string_t *str, const char *delim) {
    string_result_t result = {0};
    string_result_t tmp_res = {0};

    if (str == NULL || delim == NULL || delim[0] == '\0') {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid strings");

        return result;
    }

    const char *ptr = str->data;
    const size_t delim_len = strlen(delim);
    size_t count = 1;

    while ((ptr = strstr(ptr, delim))) {
        count++;
        ptr += delim_len;
    }

    string_t **string_array = malloc(count * sizeof(string_t *));
    if (string_array == NULL) {
        result.status = STRING_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory");

        return result;
    }

    const char *start = str->data;
    size_t idx = 0;

    while ((ptr = strstr(start, delim))) {
        const size_t part_len = ptr - start;
        char *tmp = malloc(part_len + 1);
        if (tmp == NULL) {
            result.status = STRING_ERR_ALLOCATE;
            SET_MSG(result, "Cannot allocated memory");

            goto cleanup;
        }

        memcpy(tmp, start, part_len);
        tmp[part_len] = '\0';

        tmp_res = string_new(tmp);
        free(tmp);
        if (tmp_res.status != STRING_OK) { result = tmp_res; goto cleanup; }
        
        string_array[idx++] = tmp_res.value.string;
        start = ptr + delim_len;
    }

    tmp_res = string_new(start);
    if (tmp_res.status != STRING_OK) { result = tmp_res; goto cleanup; }

    string_array[idx] = tmp_res.value.string;

    result.status = STRING_OK;
    result.value.split.strings = string_array;
    result.value.split.count = count;
    SET_MSG(result, "String successfully split");

    return result;
cleanup:
    for (size_t j = 0; j < idx; j++) {
        string_destroy(string_array[j]);
    }
    free(string_array);

    return result;
}

/**
 * string_destroy
 *  @str: a non-null string
 *
 *  Destroys @str
 *
 *  Returns a string_result_t data type
 */
string_result_t string_destroy(string_t *str) {
    string_result_t result = {0};

    if (str == NULL) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid string");

        return result;
    }

    free(str->data);
    free(str);

    result.status = STRING_OK;
    SET_MSG(result, "String successfully deleted");

    return result;
}

/**
 * string_split_destory
 *  @split: an array of pointers of String
 *  @count: the number of elements
 *
 *  Destroys the @split array of Strings
 *
 *  Returns a string_result_t data type
 */
string_result_t string_split_destroy(string_t **split, size_t count) {
    string_result_t result = {0};

    if (split == NULL) {
        result.status = STRING_ERR_INVALID;
        SET_MSG(result, "Invalid string");

        return result;
    }

    for (size_t idx = 0; idx < count; idx++) {
        string_destroy(split[idx]);
    }

    free(split);

    result.status = STRING_OK;
    SET_MSG(result, "Array of strings successfully deleted");

    return result;
}
