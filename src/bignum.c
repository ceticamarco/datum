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
static bigint_result_t bigint_compare_abs(const bigint_t *x, const bigint_t *y);
static bigint_result_t bigint_add_abs(const bigint_t *x, const bigint_t *y);
static bigint_result_t bigint_sub_abs(const bigint_t *x, const bigint_t *y);

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
 * bigint_compare_abs
 *  @x: a non-null big integer
 *  @y: a non-null big integer
 *
 *  Compares absolute value of two big integers
 *  if |x| < |y| => -1
 *  if |x| == |y| => 0
 *  if |x| > |y| => 1
 *
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_compare_abs(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    const size_t x_size = vector_size(x->digits);
    const size_t y_size = vector_size(y->digits);

    if (x_size != y_size) {
        result.value.compare_status = (x_size > y_size) ? 1 : -1;
        result.status = BIGNUM_OK;
        SET_MSG(result, "Big integer comparison was successful");

        return result;
    }

    // Start to compare from the MSB
    for (int idx = (x_size - 1); idx >= 0; idx--) {
        vector_result_t x_get = vector_get(x->digits, idx);
        if (x_get.status != VECTOR_OK) {
            vector_destroy(x->digits);
            vector_destroy(y->digits);
            result.status = BIGNUM_ERR_INVALID;
            COPY_MSG(result, x_get.message);

            return result;
        }

        vector_result_t y_get = vector_get(y->digits, idx);
        if (y_get.status != VECTOR_OK) {
            vector_destroy(x->digits);
            vector_destroy(y->digits);
            result.status = BIGNUM_ERR_INVALID;
            COPY_MSG(result, y_get.message);

            return result;
        }

        int *x_digit = (int*)x_get.value.element;
        int *y_digit = (int*)y_get.value.element;

        if (*x_digit != *y_digit) {
            result.value.compare_status = (*x_digit > *y_digit) ? 1 : -1;
            result.status = BIGNUM_OK;
            SET_MSG(result, "Big integer comparison was successful");

            return result;
        }
    }

    result.value.compare_status = 0;
    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integer comparison was successful");

    return result;
}

/**
 * bigint_compare
 *  @x: a valid non-null big integer
 *  @y: a valid non-null big integer
 * 
 *  Compares two big integers
 *  if x < y => -1
 *  if x == y => 0
 *  if x > y => 1
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_compare(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    if (x->is_negative != y->is_negative) {
        result.value.compare_status = x->is_negative ? -1 : 1;
        result.status = BIGNUM_OK;
        SET_MSG(result, "Big integer comparison was successful");

        return result;
    }
    
    bigint_result_t cmp_res = bigint_compare_abs(x, y);
    if (cmp_res.status != BIGNUM_OK) {
        vector_destroy(x->digits);
        vector_destroy(y->digits);

        return cmp_res;
    }
    
    const uint8_t abs_cmp = cmp_res.value.compare_status;

    result.value.compare_status = x->is_negative ? -abs_cmp : abs_cmp;
    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integer comparison was successful");

    return result;
}

/**
 * bigint_add_abs
 *  @x: a non-null big integer
 *  @y: a non-null big integer
 * 
 *  Adds two absolute values together
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_add_abs(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    bigint_t *sum = malloc(sizeof(bigint_t));
    if (sum == NULL) {
        result.status = BIGNUM_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory for big integer");

        return result;
    }

    const size_t max_size = vector_size(x->digits) > vector_size(y->digits) ?
                            vector_size(x->digits) : vector_size(y->digits);

    vector_result_t vec_res = vector_new(max_size + 1, sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(sum);
        result.status = BIGNUM_ERR_INVALID;
        COPY_MSG(result, vec_res.message);

        return result;
    }

    sum->digits = vec_res.value.vector;
    sum->is_negative = false;

    long long carry = 0;
    size_t idx = 0;

    while (idx < vector_size(x->digits) || idx < vector_size(y->digits) || carry) {
        long long partial_sum = carry;

        if (idx < vector_size(x->digits)) {
            vector_result_t get_res = vector_get(x->digits, idx);
            if (get_res.status != VECTOR_OK) {
                vector_destroy(sum->digits);
                free(sum);
                result.status = BIGNUM_ERR_INVALID;
                COPY_MSG(result, get_res.message);

                return result;
            }

            int *x_digit = (int*)get_res.value.element;
            partial_sum += *x_digit;
        }

        if (idx < vector_size(y->digits)) {
            vector_result_t get_res = vector_get(y->digits, idx);
            if (get_res.status != VECTOR_OK) {
                vector_destroy(sum->digits);
                free(sum);
                result.status = BIGNUM_ERR_INVALID;
                COPY_MSG(result, get_res.message);

                return result;
            }

            int *y_digit = (int*)get_res.value.element;
            partial_sum += *y_digit;
        }

        int digit = partial_sum % BIGINT_BASE;
        carry = partial_sum / BIGINT_BASE;

        vector_result_t push_res = vector_push(sum->digits, &digit);
        if (push_res.status != VECTOR_OK) {
            vector_destroy(sum->digits);
            free(sum);
            result.status = BIGNUM_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
        idx++;
    }

    bigint_result_t trim_res = bigint_trim_zeros(sum);
    if (trim_res.status != BIGNUM_OK) {
        vector_destroy(sum->digits);
        free(sum);
        
        return trim_res;
    }

    result.value.number = sum;
    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integers successfully added");

    return result;
}

/**
 * bigint_sub_abs
 *  @x: a non-null big integer
 *  @y: a non-null big integer
 * 
 *  Subtracts two absolute values assuming that |x| >= |y|
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_sub_abs(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    bigint_t *difference = malloc(sizeof(bigint_t));
    if (difference == NULL) {
        result.status = BIGNUM_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory for big integer");

        return result;
    }

    vector_result_t vec_res = vector_new(vector_size(x->digits), sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(difference);
        result.status = BIGNUM_ERR_INVALID;
        COPY_MSG(result, vec_res.message);

        return result;
    }

    difference->digits = vec_res.value.vector;
    difference->is_negative = false;

    long long borrow = 0;

    for (size_t idx = 0; idx < vector_size(x->digits); idx++) {
        vector_result_t x_get_res = vector_get(x->digits, idx);
        if (x_get_res.status != VECTOR_OK) {
            vector_destroy(difference->digits);
            free(difference);
            result.status = BIGNUM_ERR_INVALID;
            COPY_MSG(result, x_get_res.message);

            return result;
        }

        int *x_digit = (int*)x_get_res.value.element;
        long long partial_difference = *x_digit - borrow;

        if (idx < vector_size(y->digits)) {
            vector_result_t y_get_res = vector_get(y->digits, idx);
            if (y_get_res.status != VECTOR_OK) {
                vector_destroy(difference->digits);
                free(difference);
                result.status = BIGNUM_ERR_INVALID;
                COPY_MSG(result, y_get_res.message);

                return result;
            }

            int *y_digit = (int*)y_get_res.value.element;
            partial_difference -= *y_digit;
        }

        if (partial_difference < 0) {
            partial_difference += BIGINT_BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }

        int digit = partial_difference;
        vector_result_t push_res = vector_push(difference->digits, &digit);
        if (push_res.status != VECTOR_OK) {
            vector_destroy(difference->digits);
            free(difference);
            result.status = BIGNUM_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
    }

    bigint_result_t trim_res = bigint_trim_zeros(difference);
    if (trim_res.status != BIGNUM_OK) {
        vector_destroy(difference->digits);
        free(difference);
        
        return trim_res;
    }

    result.value.number = difference;
    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integers successfully subtracted");

    return result;
}

/**
 * bigint_add
 *  @x: a non-null big integer
 *  @y: a non-null big integer
 * 
 *  Adds two big integers together
 * 
 *  Returns a bigint_result_t data type 
 */
bigint_result_t bigint_add(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    if (x == NULL || y == NULL) {
        result.status = BIGNUM_ERR_INVALID;
        SET_MSG(result, "Invalid big integers");

        return result;
    }

    // Same sign: add absolute values
    if (x->is_negative == y->is_negative) {
        bigint_result_t sum_res = bigint_add_abs(x, y);
        if (sum_res.status != BIGNUM_OK) {
            return sum_res;
        }

        bigint_t *sum = sum_res.value.number;
        if (sum) {
            sum->is_negative = x->is_negative;
        }

        result.value.number = sum;
        result.status = BIGNUM_OK;
        SET_MSG(result, "Big integers successfully added");

        return result;
    }

    // Different signs: subtract smaller from larger
    bigint_result_t cmp_res = bigint_compare_abs(x, y);
    if (cmp_res.status != BIGNUM_OK) {
        return cmp_res;
    }
    
    const uint8_t cmp = cmp_res.value.compare_status;
    if (cmp == 0) {
        return bigint_from_int(0);
    } else if (cmp > 0) {
        bigint_result_t sub_res = bigint_sub_abs(x, y);
        if (sub_res.status != BIGNUM_OK) {
            return sub_res;
        }

        bigint_t *sub = sub_res.value.number;
        if (sub) {
            sub->is_negative = x->is_negative;
        }

        result.value.number = sub;
        result.status = BIGNUM_OK;
        SET_MSG(result, "Big integers successfully added");
    } else {
        bigint_result_t sub_res = bigint_sub_abs(y, x);
        if (sub_res.status != BIGNUM_OK) {
            return sub_res;
        }
        
        bigint_t *sub = sub_res.value.number;
        if (sub) {
            sub->is_negative = y->is_negative;
        }

        result.value.number = sub;
        result.status = BIGNUM_OK;
        SET_MSG(result, "Big integers successfully added");
    }

    return result;
}

/**
 * bigint_sub
 *  @x: a non-null big integer
 *  @y: a non-null big integer
 * 
 *  Subtracts two big integers together
 * 
 *  Returns a bigint_result_t data type 
 */
bigint_result_t bigint_sub(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    if (x == NULL || y == NULL) {
        result.status = BIGNUM_ERR_INVALID;
        SET_MSG(result, "Invalid big integers");

        return result;
    }

    /* To subtract two big integers we can consider
     * the following equivalence:
     * x - y = x + (-y) 
     */
    bigint_result_t neg_y_res = bigint_clone(y);
    if (neg_y_res.status != BIGNUM_OK) {
        return neg_y_res;
    }
    
    bigint_t *neg_y = neg_y_res.value.number;
    neg_y->is_negative = !neg_y->is_negative;

    bigint_result_t difference_res = bigint_add(x, neg_y);
    if (difference_res.status != BIGNUM_OK) {
        return difference_res;
    }
    
    bigint_destroy(neg_y);
    bigint_t *difference = difference_res.value.number;

    result.value.number = difference;
    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integers successfully subtracted");

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

    bigint_result_t trim_res = bigint_trim_zeros(number);
    if (trim_res.status != BIGNUM_OK) {
        vector_destroy(number->digits);
        free(number);

        return trim_res;
    }

    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integer successfully created");

    return result;
}

/**
 * bigint_clone
 *  @number: a valid non-null big integer
 *  
 *  Clones a big integer
 * 
 *  Returns a bigint_result_t data type containing the new big integer
 */
bigint_result_t bigint_clone(const bigint_t *number) {
    bigint_result_t result = {0};

    if (number == NULL) {
        result.status = BIGNUM_ERR_INVALID;
        SET_MSG(result, "Invalid big integer");

        return result;
    }

    bigint_t *cloned = malloc(sizeof(bigint_t));
    if (cloned == NULL) {
        result.status = BIGNUM_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for big integer");

        return result;
    }

    vector_result_t vec_res = vector_new(vector_size(number->digits), sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(cloned);
        result.status = BIGNUM_ERR_ALLOCATE;
        COPY_MSG(result, vec_res.message);

        return result;
    }

    result.value.number = cloned;
    result.status = BIGNUM_OK;
    SET_MSG(result, "Big integer successfully cloned");

    return result;
}

/**
 * bigint_destroy
 *  @number: a valid non-null big integer
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