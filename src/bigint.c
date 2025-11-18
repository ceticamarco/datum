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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

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
static bigint_result_t bigint_div(const bigint_t *x, const bigint_t *y);

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

    // Discard the sign since we don't need it anymore
    unsigned long long abs_val = value < 0 ? -(unsigned long long)value : (unsigned long long)value;

    if(abs_val == 0) {
        int zero = 0;
        vector_result_t push_res = vector_push(number->digits, &zero);
        if (push_res.status != VECTOR_OK) {
            vector_destroy(number->digits);
            free(number);
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            return result;
        }
    } else {
        while (abs_val != 0) {
            int digit = abs_val % BIGINT_BASE;
            vector_result_t push_res = vector_push(number->digits, &digit);
            if (push_res.status != VECTOR_OK) {
                vector_destroy(number->digits);
                free(number);
                result.status = BIGINT_ERR_INVALID;
                COPY_MSG(result, push_res.message);

                return result;
            }

            abs_val /= BIGINT_BASE;
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
    const size_t sz = vector_size(number->digits);
    for (size_t idx = 0; idx < sz; idx++) {
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

    const size_t x_size = vector_size(x->digits);
    const size_t y_size = vector_size(y->digits);
    while (idx < x_size || idx < y_size || carry) {
        long long partial_sum = carry;

        if (idx < x_size) {
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

        if (idx < y_size) {
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

    const size_t x_size = vector_size(x->digits);
    const size_t y_size = vector_size(y->digits);
    for (size_t idx = 0; idx < x_size; idx++) {
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

        if (idx < y_size) {
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
 * bigint_divmod
 *  @x: a valid non-null big integer
 *  @y: a valid non-null big integer
 * 
 *  Computes division with remainder
 *  
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_divmod(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};
    bigint_result_t tmp_res = {0};

    // Intermediate results
    bigint_t *quotient = NULL;
    bigint_t *y_times_q = NULL;
    bigint_t *remainder = NULL;

    if (x == NULL || y == NULL) {
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big numbers");

        return result;
    }

    // Check for division by zero
    const size_t y_size = vector_size(y->digits);
    if (y_size == 0) {
        result.status = BIGINT_ERR_DIV_BY_ZERO;
        SET_MSG(result, "Division by zero");

        return result;
    }

    if (y_size == 1) {
        vector_result_t y_val_res = vector_get(y->digits, 0);
        if (y_val_res.status != VECTOR_OK) {
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, y_val_res.message);

            return result;
        }

        int *y_val = (int*)y_val_res.value.element;
        if (*y_val == 0) {
            result.status = BIGINT_ERR_DIV_BY_ZERO;
            SET_MSG(result, "Division by zero");

            return result;
        }
    }

    // |x| < |y| then quotient is 0 and remainder is x
    tmp_res = bigint_compare_abs(x, y);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }

    if (tmp_res.value.compare_status < 0) {
        tmp_res = bigint_from_int(0);
        if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
        quotient = tmp_res.value.number;

        tmp_res = bigint_clone(x);
        if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
        remainder = tmp_res.value.number;

        result.value.division.quotient = quotient;
        result.value.division.remainder = remainder;
        result.status = BIGINT_OK;
        SET_MSG(result, "Division between big integers was successful");

        return result;
    }

    tmp_res = bigint_div(x, y);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    quotient = tmp_res.value.number;

    // Compute r = x - y * q
    tmp_res = bigint_prod(y, quotient);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    y_times_q = tmp_res.value.number;

    tmp_res = bigint_sub(x, y_times_q);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    remainder = tmp_res.value.number;

    // Ensure that remainder has correct sign (i.e., same as dividend x)
    // In C-style division, sign(remainder) = sign(dividend)
    remainder->is_negative = x->is_negative;

    tmp_res = bigint_trim_zeros(remainder);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
    
    result.value.division.quotient = quotient;
    result.value.division.remainder = remainder;
    result.status = BIGINT_OK;
    SET_MSG(result, "Division between big integers was successful");

    bigint_destroy(y_times_q);
    
    return result;

cleanup:
    if (quotient) { bigint_destroy(quotient); }
    if (y_times_q) { bigint_destroy(y_times_q); }
    if (remainder) { bigint_destroy(remainder); }

    return result;
}

/**
 * bigint_mod
 *  @x: a valid non-null big integer
 *  @y: a valid non-null big integer
 * 
 *  Computes @x mod @y
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_mod(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};

    if (x == NULL || y == NULL) {
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big numbers");

        return result;
    }

    bigint_result_t div_res = bigint_divmod(x, y);
    if (div_res.status != BIGINT_OK) { return div_res; }

    bigint_t* const quotient = div_res.value.division.quotient;
    bigint_t* const remainder = div_res.value.division.remainder;

    // Discard quotient
    bigint_destroy(quotient);

    result.value.number = remainder;
    result.status = BIGINT_OK;
    SET_MSG(result, "Division between big integers was successful");

    return result;
}

/**
 * bigint_shift_left
 *  @num: a non-null big integer
 *  @n: number of digits to shift
 * 
 *  Shifts left by @n digits (i.e., multiply by BASE^n)
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
    const size_t num_size = vector_size(num->digits);
    for (size_t idx = 0; idx < num_size; idx++) {
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
    bigint_result_t tmp_res = {0};

    if (x == NULL || y == NULL) {
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big integers");

        return result;
    }

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

    // Split x = x1 * BASE^pivot + x0
    tmp_res = bigint_split(x, pivot, &x1, &x0);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }

    // Split y = y1 * BASE^pivot + y0
    tmp_res = bigint_split(y, pivot, &y1, &y0);
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

    // Destroy intermediate allocations except for the product
    bigint_destroy(x1); bigint_destroy(x0);
    bigint_destroy(y1); bigint_destroy(y0);
    bigint_destroy(z0); bigint_destroy(z2);
    bigint_destroy(x_sum); bigint_destroy(y_sum);
    bigint_destroy(z1_temp); bigint_destroy(z1_sub1);
    bigint_destroy(z1); bigint_destroy(z2_shifted);
    bigint_destroy(z1_shifted); bigint_destroy(temp);

    result.value.number = product;
    result.status = BIGINT_OK;
    SET_MSG(result, "Product between big integers was successful");

cleanup: // Destroy intermediate allocations on error
    if (x1) { bigint_destroy(x1); }
    if (x0) { bigint_destroy(x0); }
    if (y1) { bigint_destroy(y1); }
    if (y0) { bigint_destroy(y0); }
    if (z0) { bigint_destroy(z0); }
    if (z2) { bigint_destroy(z2); }
    if (x_sum) { bigint_destroy(x_sum); }
    if (y_sum) { bigint_destroy(y_sum); }
    if (z1_temp) { bigint_destroy(z1_temp); }
    if (z1_sub1) { bigint_destroy(z1_sub1); }
    if (z1) { bigint_destroy(z1); }
    if (z2_shifted) { bigint_destroy(z2_shifted); }
    if (z1_shifted) { bigint_destroy(z1_shifted); }
    if (temp) { bigint_destroy(temp); }
    if (product) { bigint_destroy(product); }

    return result;
}

/**
 * bigint_dev
 *  @x: a valid non-null big integer (dividend)
 *  @y: a valid non-null big integer (divisor)
 *  
 *  Computes division using long division algorithm in O(n^2)
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_div(const bigint_t *x, const bigint_t *y) {
    bigint_result_t result = {0};
    bigint_result_t tmp_res = {0};

    bigint_t *quotient = NULL;
    bigint_t *remainder = NULL;
    bigint_t *abs_y = NULL;

    if (x == NULL || y == NULL) {
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid big numbers");

        return result;
    }

    // Check for division by zero
    const size_t y_size = vector_size(y->digits);
    if (y_size == 0) {
        result.status = BIGINT_ERR_DIV_BY_ZERO;
        SET_MSG(result, "Cannot divide by zero");

        return result;
    }

    if (y_size == 1) {
        vector_result_t y_val_res = vector_get(y->digits, 0);
        if (y_val_res.status != VECTOR_OK) {
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, y_val_res.message);

            return result;
        }

        int *y_val = (int*)y_val_res.value.element;
        if (*y_val == 0) {
            result.status = BIGINT_ERR_DIV_BY_ZERO;
            SET_MSG(result, "Cannot divide by zero");

            return result;
        }
    }

    // If |x| < |y| then result is zero
    tmp_res = bigint_compare_abs(x, y);
    if (tmp_res.status != BIGINT_OK) { return tmp_res; }

    if (tmp_res.value.compare_status < 0) {
        tmp_res = bigint_from_int(0);
        if (tmp_res.status != BIGINT_OK) { return tmp_res; }

        result.value.number = tmp_res.value.number;
        result.status = BIGINT_OK;
        SET_MSG(result, "Division between big integers was successful");

        return result;
    }

    // Initialize quotient and remainder
    tmp_res = bigint_from_int(0);
    if (tmp_res.status != BIGINT_OK) { return tmp_res; }
    quotient = tmp_res.value.number;

    tmp_res = bigint_from_int(0);
    if (tmp_res.status != BIGINT_OK) { bigint_destroy(quotient); return tmp_res; }
    remainder = tmp_res.value.number;

    // Create absolute value of y for later comparisons
    tmp_res = bigint_clone(y);
    if (tmp_res.status != BIGINT_OK) {
        bigint_destroy(quotient);
        bigint_destroy(remainder);

        return tmp_res;
    }

    abs_y = tmp_res.value.number;
    abs_y->is_negative = false;

    // Long division algorithm applied from MSB to LSB
    const size_t x_size = vector_size(x->digits);
    for (int idx = (int)x_size - 1; idx >= 0; idx--) {
        // Shift remainder left by one base digit (multiplication by BASE)
        tmp_res = bigint_shift_left(remainder, 1);
        if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }

        bigint_t *shifted_remainder = tmp_res.value.number;
        bigint_destroy(remainder);
        remainder = shifted_remainder;

        // Add current digit of 'x' to the least significant position of remainder
        vector_result_t digit_res = vector_get(x->digits, idx);
        if (digit_res.status != VECTOR_OK) {
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, digit_res.message);

            goto cleanup;
        }

        int *x_digit = (int*)digit_res.value.element;

        vector_result_t set_res = vector_set(remainder->digits, 0, x_digit);
        if (set_res.status != VECTOR_OK) {
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, set_res.message);

            goto cleanup;
        }

        tmp_res = bigint_trim_zeros(remainder);
        if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }

        // COunt how many times 'y' fits into current remainder
        size_t count = 0;
        while (1) {
            tmp_res = bigint_compare_abs(remainder, abs_y);
            if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }
            if (tmp_res.value.compare_status < 0) { break; } // remainder < abs_y

            // remainder = remainder - abs_y
            tmp_res = bigint_sub_abs(remainder, abs_y);
            if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }

            bigint_t *new_remainder = tmp_res.value.number;
            bigint_destroy(remainder);
            remainder = new_remainder;
            count++;
        }

        // Add count to quotient digits
        vector_result_t push_res = vector_push(quotient->digits, &count);
        if (push_res.status != VECTOR_OK) {
            result.status = BIGINT_ERR_INVALID;
            COPY_MSG(result, push_res.message);

            goto cleanup;
        }
    }

    // Reverse quotient digits
    const size_t q_size = vector_size(quotient->digits);
    for (size_t idx = 0; idx < q_size / 2; idx++) {
        vector_result_t left_res = vector_get(quotient->digits, idx);
        vector_result_t right_res = vector_get(quotient->digits, q_size - 1 - idx);

        if (left_res.status != VECTOR_OK || right_res.status != VECTOR_OK) {
            result.status = BIGINT_ERR_INVALID;
            SET_MSG(result, "Failed to access vector elements");

            goto cleanup;
        }

        int *left = (int*)left_res.value.element;
        int *right = (int*)right_res.value.element;
        int temp = *left;

        vector_set(quotient->digits, idx, right);
        vector_set(quotient->digits, q_size - 1 - idx, &temp);
    }

    quotient->is_negative = (x->is_negative != y->is_negative);

    tmp_res = bigint_trim_zeros(quotient);
    if (tmp_res.status != BIGINT_OK) { result = tmp_res; goto cleanup; }

    bigint_destroy(remainder);
    bigint_destroy(abs_y);

    result.value.number = quotient;
    result.status = BIGINT_OK;
    SET_MSG(result, "Division between big integers was successful");

    return result;

cleanup:
    if (quotient) { bigint_destroy(quotient); }
    if (remainder) { bigint_destroy(remainder); }
    if (abs_y) { bigint_destroy(abs_y); }

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
 * bigint_printf
 *  @format: format string
 *  @...: variadic arguments
 * 
 *  Prints a bigint integer to stdout using the custom '%B' placeholder
 * 
 *  Returns a bigint_result_t data type
 */
bigint_result_t bigint_printf(const char *format, ...) {
    bigint_result_t result = {0};

    if (format == NULL) {
        result.status = BIGINT_ERR_INVALID;
        SET_MSG(result, "Invalid format string");

        return result;
    }

    va_list args;
    va_start(args, format);

    // Process string char by char
    for (const char *p = format; *p != '\0'; p++) {
        if (*p == '%' && *(p + 1) == 'B') {
            // Process a big number
            bigint_t *num = va_arg(args, bigint_t*);
            if (num == NULL) {
                printf("<invalid string>");
            } else {
                bigint_result_t num_str_res = bigint_to_string(num);
                if (num_str_res.status != BIGINT_OK) { 
                    va_end(args); 
                    return num_str_res; 
                }
                
                char* const number_str = num_str_res.value.string_num;
                printf("%s", number_str);
                free(number_str);
            }
            p++;
        } else if (*p == '%' && *(p + 1) != '%') {
            // Handle common printf placeholders
            p++;
            char placeholder = *p;

            switch (placeholder) {
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    printf("%d", val);

                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    printf("%u", val);

                    break;
                }
                case 'l': {
                    if (*(p + 1) == 'd' || *(p + 1) == 'i') {
                        long val = va_arg(args, long);
                        printf("%ld", val);
                        p++;
                    } else if (*(p + 1) == 'l' && (*(p + 2) == 'd' || *(p + 2) == 'i')) {
                        long long val = va_arg(args, long long);
                        printf("%lld", val);
                        p += 2;
                    } else if (*(p + 1) == 'u') {
                        unsigned long val = va_arg(args, unsigned long);
                        printf("%lu", val);
                        p++;
                    }
                    break;
                }
                case 's': {
                    char *val = va_arg(args, char*);
                    printf("%s", val ? val : "<invalid string>");
                    break;
                }
                case 'c': {
                    int val = va_arg(args, int);
                    printf("%c", val);
                    break;
                }
                case 'f': {
                    double val = va_arg(args, double);
                    printf("%f", val);
                    break;
                }
                case 'p': {
                    void *val = va_arg(args, void*);
                    printf("%p", val);
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    printf("%x", val);
                    break;
                }
                case 'X': {
                    unsigned int val = va_arg(args, unsigned int);
                    printf("%X", val);
                    break;
                }
                default: // Unsupported placeholder so we just print it
                    printf("%%%c", placeholder);
                    break;
            }
        } else if (*p == '%' && *(p + 1) == '%') {
            // print the percent character as is
            putchar('%');
            p++;
        } else { // Print ASCII character
            putchar(*p);
        }
    }
    
    va_end(args);

    result.status = BIGINT_OK;
    SET_MSG(result, "Printf completed successfully");

    return result;
}
