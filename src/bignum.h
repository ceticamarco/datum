#ifndef BIGNUM_H
#define BIGNUM_H

#define RESULT_MSG_SIZE 64

// Big numbers numerical base (10^9)
#define BIGINT_BASE 1000000000
// Each digit stores values from 0 to 999,999,999
#define BIGINT_BASE_DIGITS 9

#include <stdint.h>
#include "vector.h"

typedef enum { false = 0x0, true } bool;

typedef enum {
    BIGNUM_OK = 0x0,
    BIGNUM_ERR_ALLOCATE,
    BIGNUM_ERR_DIV_BY_ZERO,
    BIGNUM_ERR_INVALID
} bignum_status_t;

typedef struct {
    vector_t *digits;
    bool is_negative;
} bigint_t;

typedef struct {
    bignum_status_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        bigint_t *number;
        char *string_num;
    } value;
} bigint_result_t;

#ifdef __cplusplus
extern "C" {
#endif

bigint_result_t bigint_from_int(long long value);
bigint_result_t bigint_from_string(const char *string_num);
bigint_result_t bigint_destroy(bigint_t *number);

#ifdef __cplusplus
}
#endif

#endif