#ifndef BIGINT_H
#define BIGINT_H

#define RESULT_MSG_SIZE 64

// Big numbers numerical base (10^9)
#define BIGINT_BASE 1000000000
// Each digit stores values from 0 to 999,999,999
#define BIGINT_BASE_DIGITS 9

#include <stdint.h>
#include <stdbool.h>
#include "vector.h"

typedef enum {
    BIGINT_OK = 0x0,
    BIGINT_ERR_ALLOCATE,
    BIGINT_ERR_DIV_BY_ZERO,
    BIGINT_ERR_INVALID
} bigint_status_t;

typedef struct {
    vector_t *digits;
    bool is_negative;
} bigint_t;

typedef struct {
    bigint_t *quotient;
    bigint_t *remainder;
} div_result_t;

typedef struct {
    bigint_status_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        bigint_t *number;
        div_result_t division;
        int8_t compare_status;
        char *string_num;
    } value;
} bigint_result_t;

#ifdef __cplusplus
extern "C" {
#endif

bigint_result_t bigint_from_int(long long value);
bigint_result_t bigint_from_string(const char *string_num);
bigint_result_t bigint_to_string(const bigint_t *number);
bigint_result_t bigint_clone(const bigint_t *number);
bigint_result_t bigint_compare(const bigint_t *x, const bigint_t *y);
bigint_result_t bigint_add(const bigint_t *x, const bigint_t *y);
bigint_result_t bigint_sub(const bigint_t *x, const bigint_t *y);
bigint_result_t bigint_prod(const bigint_t *x, const bigint_t *y);
bigint_result_t bigint_divmod(const bigint_t *x, const bigint_t *y);
bigint_result_t bigint_mod(const bigint_t *x, const bigint_t *y);
bigint_result_t bigint_destroy(bigint_t *number);
bigint_result_t bigint_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif