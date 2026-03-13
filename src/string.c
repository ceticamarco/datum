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

// Get byte length of an UTF-8 sequence
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

    const size_t len = utf8_char_len((unsigned char)utf8_char[0]);
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

// Validate an UTF-8 symbol and measure byte length and character count
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
        p_size += len;
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
        out[1] = (char)(0x80 | (codepount & 0x3F));

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
