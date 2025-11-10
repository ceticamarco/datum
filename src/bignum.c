#define SET_MSG(result, msg) \
    do { \
        snprintf((char *)(result).message, RESULT_MSG_SIZE, "%s", (const char *)msg); \
    } while (0)

#define COPY_MSG(result, msg) \
    do { \
        strncpy((char *)(result).message, (const char *)(msg), RESULT_MSG_SIZE - 1); \
        (result).message[RESULT_MSG_SIZE - 1] = '\0'; \
    } while (0)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bignum.h"
#include "vector.h"

// Internal methods
static bigint_result_t bigint_trim_zeros(bigint_t *number);

/**
 * bigint_from_int
 *  @value: an integer value
 * 
 *  Takes an integer and convert it to a big integer
 * 
 *  Returns a big_int_result_t data type containing a new big integer
 */
bigint_result_t bigint_from_int(long long value) {
    bigint_result_t result = {0};

    bigint_t *number = malloc(sizeof(bigint_t));
    if (number == NULL) {
        result.status = BIGNUM_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for big integer");

        return result;
    }

    vector_result_t vec_res = vector_new(4, sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(number);
        result.status = BIGNUM_ERR_ALLOCATE;
        COPY_MSG(result, vec_res.message);

        return result;
    }

    number->digits = vec_res.value.vector;
    number->is_negative = (value < 0);

    if (value < 0) {
        value = -value;
    } else if (value == 0) {
        int zero = 0;
        vector_result_t push_res = vector_push(number->digits, &zero);
        if (push_res.status != VECTOR_OK) {
            vector_destroy(number->digits);
            free(number);
            result.status = BIGNUM_ERR_INVALID;
            COPY_MSG(result, vec_res.message);

            return result;
        }
    } else {
        while (value > 0) {
            int digit = value % BIGINT_BASE;
            vector_result_t push_res = vector_push(number->digits, &digit);
            if (push_res.status != VECTOR_OK) {
                vector_destroy(number->digits);
                free(number);
                result.status = BIGNUM_ERR_INVALID;
                COPY_MSG(result, vec_res.message);

                return result;
            }

            value /= BIGINT_BASE;
        }
    }

    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integer successfully created");
    result.value.number = number;

    return result;
}

/**
 * bigint_trim_zeros
 *  @number: a non-null big integer
 * 
 *  Helper function to remove leading zeros
 * 
 *  Returns a bigint_result_t data type 
 */
static bigint_result_t bigint_trim_zeros(bigint_t *number) {
    bigint_result_t result = {0};

    size_t number_len = vector_size(number->digits);

    while (number_len > 1) {
        vector_result_t get_res = vector_get(number->digits, number_len - 1);
        if (get_res.status != VECTOR_OK) {
            vector_destroy(number->digits);
            free(number);
            result.status = BIGNUM_ERR_INVALID;
            COPY_MSG(result, get_res.message);

            return result;
        }

        int *last = (int*)get_res.value.element;
        if (*last != 0) {
            break;
        }

        vector_result_t pop_res = vector_pop(number->digits);
        if (pop_res.status != VECTOR_OK) {
            vector_destroy(number->digits);
            free(number);
            result.status = BIGNUM_ERR_INVALID;
            COPY_MSG(result, get_res.message);

            return result;
        }
        number_len--;
    }

    if (number_len == 1) {
        vector_result_t get_res = vector_get(number->digits, number_len - 1);
        if (get_res.status != VECTOR_OK) {
            vector_destroy(number->digits);
            free(number);
            result.status = BIGNUM_ERR_INVALID;
            COPY_MSG(result, get_res.message);

            return result;
        }

        int *first = (int*)get_res.value.element;
        if (*first == 0) {
            number->is_negative = false;
        }
    }

    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integer successfully trimmed");

    return result;

}

/**
 * bigint_from_string
 *  @string_num: an array of chars representing a number
 * 
 *  Takes a string containing a number and convert it to big integer
 * 
 *  Returns a bigint_result_t data type containing a new big integer
 */
bigint_result_t bigint_from_string(const char *string_num) {
    bigint_result_t result = {0};

    if (string_num == NULL || *string_num == 0) {
        result.status = BIGNUM_ERR_INVALID;
        SET_MSG(result, "Invalid string");

        return result;
    }

    bigint_t *number = malloc(sizeof(bigint_t));
    if (number == NULL) {
        result.status = BIGNUM_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for big integer");

        return result;
    }

    vector_result_t vec_res = vector_new(4, sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        vector_destroy(number->digits);
        free(number);        
        result.status = BIGNUM_ERR_ALLOCATE;
        COPY_MSG(result, vec_res.message);

        return result;
    }

    number->digits = vec_res.value.vector;

    number->is_negative = false;
    if (*string_num == '-') {
        number->is_negative = true;
        string_num++;
    } else if (*string_num == '+') {
        string_num++;
    }

    // Check whether the integer is valid or not
    if (*string_num == '\0') {
        vector_destroy(number->digits);
        free(number);
        result.status = BIGNUM_ERR_ALLOCATE;
        SET_MSG(result, "Invalid integer");

        return result;
    }

    // Skip leading zeros
    while (*string_num == '0' && *(string_num + 1) != '\0') {
        string_num++;
    }

    const size_t number_len = strlen(string_num);

    // Process digits from right to left by chunks of the representation base
    for (int i = number_len; i > 0; i -= BIGINT_BASE_DIGITS) {
        const int start = (i - BIGINT_BASE_DIGITS > 0) ? i - BIGINT_BASE_DIGITS : 0;
        const int chunk_len = (i - start);

        int digit = 0;
        for (int j = 0; j < chunk_len; j++) {
            digit *= 10 + (string_num[start + j] - '0');
        }

        vector_result_t push_res = vector_push(number->digits, &digit);
        if (push_res.status != VECTOR_OK) {
            vector_destroy(number->digits);
            free(number);
            result.status = BIGNUM_ERR_ALLOCATE;
            COPY_MSG(result, vec_res.message);

            return result;
        }
    }

    bigint_trim_zeros(number);

    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integer successfully created");

    return result;
}

/**
 * bigint_destroy
 *  @number: a valid big integer
 * 
 *  Deletes the big integer from the memory
 *  
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_destroy(bigint_t *number) {
    bigint_result_t result = {0};

    if (number == NULL) {
        result.status = BIGNUM_ERR_INVALID;
        SET_MSG(result, "Invalid big integer");

        return result;
    }

    vector_destroy(number->digits);
    free(number);

    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integer successfully deleted");

    return result;
}