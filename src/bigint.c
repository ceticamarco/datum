#define SET_MSG(result, msg) \
    do { \
        snprintf((char *)(result).message, RESULT_MSG_SIZE, "%s", (const char *)msg); \
    } while (0)

#define COPY_MSG(result, msg) \
    do { \
        strncpy((char *)(result).message, (const char *)(msg), RESULT_MSG_SIZE - 1); \
        (result).message[RESULT_MSG_SIZE - 1] = '\0'; \
    } while (0)

#define IS_DIGIT(c) ((c) >= '0') && ((c) <= '9')

#define DESTROY_IF(p) \
    do { \
        if ((p) && (p) != result.value.number) { \
            bigint_destroy((p)); \
            (p) = NULL; \
        } \
    } while (0)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bigint.h"
#include "vector.h"

// Internal methods
static bigint_result_t bigint_trim_zeros(bigint_t *number);
static bigint_result_t bigint_compare_abs(const bigint_t *x, const bigint_t *y);
static bigint_result_t bigint_add_abs(const bigint_t *x, const bigint_t *y);
static bigint_result_t bigint_sub_abs(const bigint_t *x, const bigint_t *y);
static bigint_result_t bigint_shift_left(const bigint_t *num, size_t n);
static bigint_result_t bigint_split(const bigint_t *num, size_t m, bigint_t **high, bigint_t **low);
static bigint_result_t bigint_karatsuba_base(const bigint_t *x, const bigint_t *y);
static bigint_result_t bigint_karatsuba(const bigint_t *x, const bigint_t *y);

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
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for big integer");

        return result;
    }

    vector_result_t vec_res = vector_new(4, sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(number);
        result.status = BIGINT_ERR_ALLOCATE;
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
            result.status = BIGINT_ERR_INVALID;
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
                result.status = BIGINT_ERR_INVALID;
                COPY_MSG(result, vec_res.message);

                return result;
            }

            value /= BIGINT_BASE;
        }
    }

    result.status = BIGINT_OK;
    SET_MSG(result, "Big integer successfully created");
    result.value.number = number;

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
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid string");

        return result;
    }

    bigint_t *number = malloc(sizeof(bigint_t));
    if (number == NULL) {
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for big integer");

        return result;
    }

    vector_result_t vec_res = vector_new(4, sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(number);        
        result.status = BIGINT_ERR_ALLOCATE;
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
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Invalid integer");

        return result;
    }

    // Check whether characters are digits
    for (const char *p = string_num; *p; ++p) {
        if (!IS_DIGIT((unsigned char)*p)) {
            vector_destroy(number->digits);
            free(number);
            result.status = BIGINT_ERR_INVALID;
            SET_MSG(result, "Invalid integer");

            return result;
        }
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
            // digit *= 10 + (string_num[start + j] - '0');
            digit = digit * 10 + (string_num[start + j] - '0');
        }

        vector_result_t push_res = vector_push(number->digits, &digit);
        if (push_res.status != VECTOR_OK) {
            vector_destroy(number->digits);
            free(number);
            result.status = BIGINT_ERR_ALLOCATE;
            COPY_MSG(result, push_res.message);

            return result;
        }
    }

    bigint_result_t trim_res = bigint_trim_zeros(number);
    if (trim_res.status != BIGINT_OK) {
        vector_destroy(number->digits);
        free(number);

        return trim_res;
    }

    result.value.number = number;
    result.status = BIGINT_OK;
    SET_MSG(result, "Big integer successfully created");

    return result;
}

/**
 * bigint_to_string
 *  @number: a valid non-null big number
 * 
 *  Converts a big integer to a C string
 *
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_to_string(const bigint_t *number) {
    bigint_result_t result = {0};

    if (number == NULL) {
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big integer");

        return result;
    }

    const size_t size = vector_size(number->digits);
    const size_t max_len = (size * BIGINT_BASE_DIGITS) + 2; // +2 for sign and terminator
    
    char *str = malloc(max_len);
    if (str == NULL) {
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for string");

        return result;
    }

    char *ptr = str;
    if (number->is_negative) {
        *ptr++ = '-';
    }

    // Print MSB without leading zeros
    vector_result_t msb_res = vector_get(number->digits, size - 1);
    if (msb_res.status != VECTOR_OK) {
        result.status = BIGINT_ERR_INVALID;
        COPY_MSG(result, msb_res.message);

        return result;
    }

    int *msb = (int*)msb_res.value.element;
    ptr += sprintf(ptr, "%d", *msb);

    // Print remaining digits with leading zeros
    for (int idx = size - 2; idx >= 0; idx--) {
        vector_result_t digit_res = vector_get(number->digits, idx);
        if (digit_res.status != VECTOR_OK) {
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, digit_res.message);

            return result;
        }

        int *digit = (int*)digit_res.value.element;
        ptr += sprintf(ptr, "%09d", *digit);
    }

    result.value.string_num = str;
    result.status = BIGINT_OK;
    SET_MSG(result, "Big integer successfully converted");

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
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big integer");

        return result;
    }

    bigint_t *cloned = malloc(sizeof(bigint_t));
    if (cloned == NULL) {
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for big integer");

        return result;
    }

    vector_result_t vec_res = vector_new(vector_size(number->digits), sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(cloned);
        result.status = BIGINT_ERR_ALLOCATE;
        COPY_MSG(result, vec_res.message);

        return result;
    }

    cloned->digits = vec_res.value.vector;
    cloned->is_negative = number->is_negative;

    // Copy digits
    for (size_t idx = 0; idx < vector_size(number->digits); idx++) {
        vector_result_t get_res = vector_get(number->digits, idx);
        if (get_res.status != VECTOR_OK) {
            vector_destroy(cloned->digits);
            free(cloned);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, get_res.message);

            return result;
        }

        int *digit = (int*)get_res.value.element;
        
        vector_result_t push_res = vector_push(cloned->digits, digit);
        if (push_res.status != VECTOR_OK) {
            vector_destroy(cloned->digits);
            free(cloned);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
    }

    result.value.number = cloned;
    result.status = BIGINT_OK;
    SET_MSG(result, "Big integer successfully cloned");

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
            result.status = BIGINT_ERR_INVALID;
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
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, get_res.message);

            return result;
        }
        number_len--;
    }

    if (number_len == 1) {
        vector_result_t get_res = vector_get(number->digits, number_len - 1);
        if (get_res.status != VECTOR_OK) {
            vector_destroy(number->digits);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, get_res.message);

            return result;
        }

        int *first = (int*)get_res.value.element;
        if (*first == 0) {
            number->is_negative = false;
        }
    }

    result.status = BIGINT_OK;
    SET_MSG(result, "Big integer successfully trimmed");

    return result;

}

/**
 * bigint_compare_abs
 *  @x: a non-null big integer
 *  @y: a non-null big integer
 *
 *  Compares absolute value of two big integers
 *  if |x| <  |y| => -1
 *  if |x| == |y| => 0
 *  if |x| >  |y| => 1
 *
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_compare_abs(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    const size_t x_size = vector_size(x->digits);
    const size_t y_size = vector_size(y->digits);

    if (x_size != y_size) {
        result.value.compare_status = (x_size > y_size) ? 1 : -1;
        result.status = BIGINT_OK;
        SET_MSG(result, "Big integer comparison was successful");

        return result;
    }

    // Start to compare from the MSB
    for (int idx = (int)(x_size - 1); idx >= 0; idx--) {
        vector_result_t x_get = vector_get(x->digits, idx);
        if (x_get.status != VECTOR_OK) {
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, x_get.message);

            return result;
        }

        vector_result_t y_get = vector_get(y->digits, idx);
        if (y_get.status != VECTOR_OK) {
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, y_get.message);

            return result;
        }

        int *x_digit = (int*)x_get.value.element;
        int *y_digit = (int*)y_get.value.element;

        if (*x_digit != *y_digit) {
            result.value.compare_status = (*x_digit > *y_digit) ? 1 : -1;
            result.status = BIGINT_OK;
            SET_MSG(result, "Big integer comparison was successful");

            return result;
        }
    }

    result.value.compare_status = 0;
    result.status = BIGINT_OK;
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
        result.status = BIGINT_OK;
        SET_MSG(result, "Big integer comparison was successful");

        return result;
    }
    
    bigint_result_t cmp_res = bigint_compare_abs(x, y);
    if (cmp_res.status != BIGINT_OK) {
        return cmp_res;
    }
    
    const int8_t abs_cmp = cmp_res.value.compare_status;

    result.value.compare_status = x->is_negative ? -abs_cmp : abs_cmp;
    result.status = BIGINT_OK;
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
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory for big integer");

        return result;
    }

    const size_t max_size = vector_size(x->digits) > vector_size(y->digits) ?
                            vector_size(x->digits) : vector_size(y->digits);

    vector_result_t vec_res = vector_new(max_size + 1, sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(sum);
        result.status = BIGINT_ERR_INVALID;
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
                result.status = BIGINT_ERR_INVALID;
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
                result.status = BIGINT_ERR_INVALID;
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
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
        idx++;
    }

    bigint_result_t trim_res = bigint_trim_zeros(sum);
    if (trim_res.status != BIGINT_OK) {
        vector_destroy(sum->digits);
        free(sum);
        
        return trim_res;
    }

    result.value.number = sum;
    result.status = BIGINT_OK;
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
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Cannot allocate memory for big integer");

        return result;
    }

    vector_result_t vec_res = vector_new(vector_size(x->digits), sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(difference);
        result.status = BIGINT_ERR_INVALID;
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
            result.status = BIGINT_ERR_INVALID;
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
                result.status = BIGINT_ERR_INVALID;
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
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
    }

    bigint_result_t trim_res = bigint_trim_zeros(difference);
    if (trim_res.status != BIGINT_OK) {
        vector_destroy(difference->digits);
        free(difference);
        
        return trim_res;
    }

    result.value.number = difference;
    result.status = BIGINT_OK;
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
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big integers");

        return result;
    }

    // Same sign: add absolute values
    if (x->is_negative == y->is_negative) {
        bigint_result_t sum_res = bigint_add_abs(x, y);
        if (sum_res.status != BIGINT_OK) {
            return sum_res;
        }

        bigint_t *sum = sum_res.value.number;
        if (sum) {
            sum->is_negative = x->is_negative;
        }

        result.value.number = sum;
        result.status = BIGINT_OK;
        SET_MSG(result, "Big integers successfully added");

        return result;
    }

    // Different signs: subtract smaller from larger
    bigint_result_t cmp_res = bigint_compare_abs(x, y);
    if (cmp_res.status != BIGINT_OK) {
        return cmp_res;
    }
    
    const int8_t cmp = cmp_res.value.compare_status;
    if (cmp == 0) {
        return bigint_from_int(0);
    } else if (cmp > 0) {
        bigint_result_t sub_res = bigint_sub_abs(x, y);
        if (sub_res.status != BIGINT_OK) {
            return sub_res;
        }

        bigint_t *sub = sub_res.value.number;
        if (sub) {
            sub->is_negative = x->is_negative;
        }

        result.value.number = sub;
        result.status = BIGINT_OK;
        SET_MSG(result, "Big integers successfully added");
    } else {
        bigint_result_t sub_res = bigint_sub_abs(y, x);
        if (sub_res.status != BIGINT_OK) {
            return sub_res;
        }
        
        bigint_t *sub = sub_res.value.number;
        if (sub) {
            sub->is_negative = y->is_negative;
        }

        result.value.number = sub;
        result.status = BIGINT_OK;
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
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big integers");

        return result;
    }

    /* To subtract two big integers we can consider
     * the following equivalence:
     * x - y = x + (-y) 
     */
    bigint_result_t neg_y_res = bigint_clone(y);
    if (neg_y_res.status != BIGINT_OK) {
        return neg_y_res;
    }
    
    bigint_t *neg_y = neg_y_res.value.number;
    neg_y->is_negative = !neg_y->is_negative;

    bigint_result_t difference_res = bigint_add(x, neg_y);
    if (difference_res.status != BIGINT_OK) {
        bigint_destroy(neg_y);

        return difference_res;
    }
    
    bigint_destroy(neg_y);
    bigint_t *difference = difference_res.value.number;

    result.value.number = difference;
    result.status = BIGINT_OK;
    SET_MSG(result, "Big integers successfully subtracted");

    return result;
}

/**
 * bigint_prod
 *  @x: a non-null big integer
 *  @y: a non-null big integer
 * 
 *  Perform a multiplication between @a and @b
 *  using Karatsuba's algorithm
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_prod(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    if (x == NULL || y == NULL) {
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big integers");

        return result;
    }

    bigint_result_t product_res = bigint_karatsuba(x, y);
    if (product_res.status != BIGINT_OK) {
        return product_res;
    }

    bigint_t *product = product_res.value.number;
    product->is_negative = (x->is_negative != y->is_negative);

    bigint_result_t trim_res = bigint_trim_zeros(product);
    if (trim_res.status != BIGINT_OK) {
        bigint_destroy(product);

        return trim_res;
    }

    result.value.number = product;
    result.status = BIGINT_OK;
    SET_MSG(result, "Product between big integers was successful");

    return result;
}

/**
 * bigint_shift_left
 *  @num: a non-null big integer
 *  @n: number of digits to shift
 * 
 *  Shift left by @n digits (i.e., multiply by BASE^n)
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_shift_left(const bigint_t *num, size_t n) {
    bigint_result_t result = {0};

    if (n == 0) {
        return bigint_clone(num);
    }

    bigint_t *shifted = malloc(sizeof(bigint_t));
    if (shifted == NULL) {
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for big integer");

        return result;
    }

    vector_result_t vec_res = vector_new(vector_size(num->digits) + n, sizeof(int));
    if (vec_res.status != VECTOR_OK) {
        free(shifted);
        result.status = BIGINT_ERR_ALLOCATE;
        COPY_MSG(result, vec_res.message);

        return result;
    }

    shifted->digits = vec_res.value.vector;
    shifted->is_negative = num->is_negative;

    // Add 'n' zeros by starting from the LSB
    int zero = 0;
    for (size_t idx = 0; idx < n; idx++) {
        vector_result_t push_res = vector_push(shifted->digits, &zero);
        if (push_res.status != VECTOR_OK) {
            vector_destroy(shifted->digits);
            free(shifted);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
    }

    // Copy back original digits
    for (size_t idx = 0; idx < vector_size(num->digits); idx++) {
        vector_result_t get_res = vector_get(num->digits, idx);
        if (get_res.status != VECTOR_OK) {
            vector_destroy(shifted->digits);
            free(shifted);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, get_res.message);

            return result;
        }

        int *digit = (int*)get_res.value.element;
        vector_result_t push_res = vector_push(shifted->digits, digit);
        if (push_res.status != VECTOR_OK) {
            vector_destroy(shifted->digits);
            free(shifted);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
    }

    result.value.number = shifted;
    result.status = BIGINT_OK;
    SET_MSG(result, "Big integer shifted successfully");
    
    return result;
}

/**
 * bigint_split
 *  @num: a non-null big integers
 *  @m: the pivot/position where to split
 *  @high: digits \in [0, m)
 *  @low: digits \in [m, size)
 * 
 *  Splits number into @high and @low parts at position @m
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_split(const bigint_t *num, size_t m, bigint_t **high, bigint_t **low) {
    bigint_result_t result = {0};

    const size_t size = vector_size(num->digits);

    // Low part: digits \in [0, m)
    *low = malloc(sizeof(bigint_t));
    if (*low == NULL) {
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for big integer");

        return result;
    }

    vector_result_t low_res = vector_new(m ? m : 1, sizeof(int));
    if (low_res.status != VECTOR_OK) {
        free(*low);
        result.status = BIGINT_ERR_ALLOCATE;
        COPY_MSG(result, low_res.message);

        return result;
    }

    (*low)->digits = low_res.value.vector;
    (*low)->is_negative = false;

    for (size_t idx = 0; idx < m && idx < size; idx++) {
        vector_result_t get_res = vector_get(num->digits, idx);
        if (get_res.status != VECTOR_OK) {
            vector_destroy((*low)->digits);
            free(*low);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, get_res.message);

            return result;
        }

        int *digit = (int*)get_res.value.element;
        vector_result_t push_res = vector_push((*low)->digits, digit);
        if (push_res.status != VECTOR_OK) {
            vector_destroy((*low)->digits);
            free(*low);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
    }

    if (vector_size((*low)->digits) == 0) {
        int zero = 0;
        vector_result_t push_res = vector_push((*low)->digits, &zero);
        if (push_res.status != VECTOR_OK) {
            vector_destroy((*low)->digits);
            free(*low);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
    }

    // First pass of zero trimming
    bigint_result_t first_trim_res = bigint_trim_zeros(*low);
    if (first_trim_res.status != BIGINT_OK) {
        vector_destroy((*low)->digits);
        free(*low);

        return first_trim_res;
    }

    // High part: digits \in [m, size)
    *high = malloc(sizeof(bigint_t));
    if (*high == NULL) {
        vector_destroy((*low)->digits);
        free(*low);
        result.status = BIGINT_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for big integer");

        return result;
    }

    vector_result_t high_res = vector_new(size > m ? (size - m) : 1, sizeof(int));
    if (high_res.status != VECTOR_OK) {
        vector_destroy((*low)->digits);
        free(*low);
        free(*high);

        result.status = BIGINT_ERR_ALLOCATE;
        COPY_MSG(result, low_res.message);

        return result;
    }

    (*high)->digits = high_res.value.vector;
    (*high)->is_negative = false;

    if (size > m) {
        for (size_t idx = m; idx < size; idx++) {
            vector_result_t get_res = vector_get(num->digits, idx);
            if (get_res.status != VECTOR_OK) {
                vector_destroy((*low)->digits);
                vector_destroy((*high)->digits);
                free(*low);
                free(*high);

                result.status = BIGINT_ERR_INVALID;
                COPY_MSG(result, get_res.message);

                return result;
            }

            int *digit = (int*)get_res.value.element;
            vector_result_t push_res = vector_push((*high)->digits, digit);
            if (push_res.status != VECTOR_OK) {
                vector_destroy((*low)->digits);
                vector_destroy((*high)->digits);
                free(*low);
                free(*high);

                result.status = BIGINT_ERR_INVALID;
                COPY_MSG(result, push_res.message);

                return result;
            }
        }
    } else {
        int zero = 0;
        vector_result_t push_res = vector_push((*high)->digits, &zero);
        if (push_res.status != VECTOR_OK) {
            vector_destroy((*low)->digits);
            vector_destroy((*high)->digits);
            free(*low);
            free(*high);

            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
    }

    // Second pass of zero trimming
    bigint_result_t second_trim_res = bigint_trim_zeros(*high);
    if (second_trim_res.status != BIGINT_OK) {
        vector_destroy((*low)->digits);
        vector_destroy((*high)->digits);
        free(*low);
        free(*high);

        return second_trim_res;
    }

    result.status = BIGINT_OK;
    SET_MSG(result, "Big number successfully splitted");

    return result;
}

/**
 * bigint_karatsuba_base
 *  @x: a non-null big integer
 *  @y: a non-null big integer
 * 
 *  Base case of the Karatsuba recursive algorithm
 *  which uses a "grade school" multiplication.
 *  Its complexity is O(n^2)
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_karatsuba_base(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    bigint_result_t prod_res = bigint_from_int(0);
    if (prod_res.status != BIGINT_OK) {
        result.status = BIGINT_ERR_ALLOCATE;
        COPY_MSG(result, prod_res.message);

        return result;        
    }

    bigint_t *product = prod_res.value.number;
    const size_t x_size = vector_size(x->digits);
    const size_t y_size = vector_size(y->digits);

    for (size_t i = 0; i < x_size; i++) {
        long long carry = 0;
        
        vector_result_t get_res = vector_get(x->digits, i);
        if (get_res.status != VECTOR_OK) {
            bigint_destroy(product);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, get_res.message);

            return result;
        }

        int *x_digit = (int*)get_res.value.element;
        for (size_t j = 0; j < y_size || carry; j++) {
            int *y_digit = NULL;
            int *curr = NULL;

            if (j < y_size) {
                vector_result_t y_res = vector_get(y->digits, j);
                if (y_res.status != VECTOR_OK) {
                    bigint_destroy(product);
                    result.status = BIGINT_ERR_INVALID;
                    COPY_MSG(result, y_res.message);

                    return result;
                }

                y_digit = (int*)y_res.value.element;
            }

            if ((i + j) < vector_size(product->digits)) {
                vector_result_t curr_res = vector_get(product->digits, i + j);
                if (curr_res.status != VECTOR_OK) {
                    bigint_destroy(product);
                    result.status = BIGINT_ERR_INVALID;
                    COPY_MSG(result, curr_res.message);

                    return result;
                }

                curr = (int*)curr_res.value.element;
            }

            long long partial_prod = carry;
            if (curr) { partial_prod += *curr; }
            if (y_digit) { partial_prod += (long long)(*x_digit) * (*y_digit); }

            int new_digit =(int)(partial_prod % BIGINT_BASE);
            carry = partial_prod / BIGINT_BASE;

            if (curr) {
                vector_result_t set_res = vector_set(product->digits, i + j, &new_digit);
                if (set_res.status != VECTOR_OK) {
                    bigint_destroy(product);
                    result.status = BIGINT_ERR_INVALID;
                    COPY_MSG(result, set_res.message);

                    return result;                    
                }
            } else {
                vector_result_t push_res = vector_push(product->digits, &new_digit);
                if (push_res.status != VECTOR_OK) {
                    bigint_destroy(product);
                    result.status = BIGINT_ERR_INVALID;
                    COPY_MSG(result, push_res.message);

                    return result;                    
                }
            }
        }
    }

    bigint_result_t trim_res = bigint_trim_zeros(product);
    if (trim_res.status != BIGINT_OK) {
        bigint_destroy(product);

        return trim_res;
    }

    result.value.number = product;
    result.status = BIGINT_OK;
    SET_MSG(result, "Product between big integers was successful");

    return result;
}

/**
 * bigint_karatusba
 *  @x: a non-null big integer
 *  @y: a non-null big integer
 * 
 *  Perform a multiplication using Karatsuba recursive algorithm
 *  in O(n^{\log_2 3}) \approx O(n^{1.585})
 */
bigint_result_t bigint_karatsuba(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    const size_t x_size = vector_size(x->digits);
    const size_t y_size = vector_size(y->digits);

    // Base case using "grade school" quadratic algorithm
    if (x_size <= 32 || y_size <= 32) {
        return bigint_karatsuba_base(x, y);
    }

    // Split the big integer at approximately half the size of the larger number
    const size_t pivot = (x_size > y_size ? x_size : y_size) / 2;

    // Results of each step
    bigint_t *x1 = NULL, *x0 = NULL;
    bigint_t *y1 = NULL, *y0 = NULL;
    bigint_t *z0 = NULL, *z2 = NULL;
    bigint_t *x_sum = NULL, *y_sum = NULL;
    bigint_t *z1_temp = NULL, *z1_sub1 = NULL, *z1 = NULL;
    bigint_t *z2_shifted = NULL, *z1_shifted = NULL;
    bigint_t *temp = NULL, *product = NULL;

    bigint_result_t tmp_res = {0};

    // Split x = x1 * BASE^pivot + x0
    tmp_res = bigint_split(x, pivot, &x1, &x0);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }

    // Split y = y1 * BASE^pivot + y0
    tmp_res = bigint_split(x, pivot, &y1, &y0);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }

    // Perform karatsuba's trick
    tmp_res = bigint_karatsuba(x0, y0); // z0 = x0 * y0
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    z0 = tmp_res.value.number;

    tmp_res = bigint_karatsuba(x1, y1); // z2 = x1 * y1
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    z2 = tmp_res.value.number;

    // z1 = (x0 + x1) * (y0 + y1) - z0 - z2
    tmp_res = bigint_add(x0, x1); // x_sum = x0 + x1
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    x_sum = tmp_res.value.number;

    tmp_res = bigint_add(y0, y1); // y_sum = y0 + y1
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    y_sum = tmp_res.value.number;

    tmp_res = bigint_karatsuba(x_sum, y_sum); // z1_temp = (x0 + x1) * (y0 + y1)
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    z1_temp = tmp_res.value.number;

    tmp_res = bigint_sub(z1_temp, z0); // z1_sub1 = z1_temp - z0
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    z1_sub1 = tmp_res.value.number;

    tmp_res = bigint_sub(z1_sub1, z2); // z1 = z1_sub1 - z2
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    z1 = tmp_res.value.number;

    tmp_res = bigint_shift_left(z2, 2 * pivot); // z2_shifted = z2 << (2 * pivot)
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    z2_shifted = tmp_res.value.number;

    tmp_res = bigint_shift_left(z1, pivot); // z1_shifted = z1 << pivot
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    z1_shifted = tmp_res.value.number;

    tmp_res = bigint_add(z2_shifted, z1_shifted);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    temp = tmp_res.value.number;

    tmp_res = bigint_add(temp, z0);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    product = tmp_res.value.number;

    result.value.number = product;
    result.status = BIGINT_OK;
    SET_MSG(result, "Product between big integers was successful");

cleanup: // Destroy intermediate allocations except for the product
    DESTROY_IF(x1); DESTROY_IF(x0);
    DESTROY_IF(y1); DESTROY_IF(y0);
    DESTROY_IF(z0); DESTROY_IF(z2);
    DESTROY_IF(x_sum); DESTROY_IF(y_sum);
    DESTROY_IF(z1_temp); DESTROY_IF(z1_sub1); DESTROY_IF(z1);
    DESTROY_IF(z2_shifted); DESTROY_IF(z1_shifted);
    DESTROY_IF(temp);

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
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big integer");

        return result;
    }

    vector_destroy(number->digits);
    free(number);

    result.status = BIGINT_OK;
    SET_MSG(result, "Big integer successfully deleted");

    return result;
}

/**
 * bigint_print
 *  @number: a valid non-null big integer
 * 
 *  Prints @number to standard output
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_print(const bigint_t *number) {
    bigint_result_t result = {0};

    bigint_result_t num_str_res = bigint_to_string(number);
    if (num_str_res.status != BIGINT_OK) {
        return num_str_res;
    }

    char *number_str = num_str_res.value.string_num;
    
    printf("%s", number_str);
    free(number_str);

    result.status = BIGINT_OK;
    SET_MSG(result, "Big integer successfully printed");

    return result;
}