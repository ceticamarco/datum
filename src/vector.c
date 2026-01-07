#define SET_MSG(result, msg) \
    do { \
        snprintf((char *)(result).message, RESULT_MSG_SIZE, "%s", (const char *)msg); \
    } while (0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

// Internal methods
/**
 * vector_resize
 *  @vector: a non-null vector
 *
 *  Increases the size of @vector
 *
 *  Returns a vector_result_t data type containing the status
 */
static vector_result_t vector_resize(vector_t *vector) {
    vector_result_t result = {0};

    const size_t old_capacity = vector->capacity;
    const size_t new_capacity = old_capacity > 0 ? old_capacity * 2 : 1;

    // Check for stack overflow errors
    if (new_capacity > SIZE_MAX / vector->data_size) {
        result.status = VECTOR_ERR_OVERFLOW;
        SET_MSG(result, "Exceeded maximum size while resizing vector");

        return result;
    }

    void *new_elements = realloc(vector->elements, new_capacity * vector->data_size);
    if (new_elements == NULL) {
        result.status = VECTOR_ERR_ALLOCATE;
        SET_MSG(result, "Failed to reallocate memory for vector");

        return result;
    }

    vector->elements = new_elements;
    vector->capacity = new_capacity;

    result.status = VECTOR_OK;
    SET_MSG(result, "Vector successfully resized");

    return result;
}

/**
 * swap
 *  @x: first element
 *  @y: second element
 * 
 *  Swaps @x and @y
 */
static void swap(void *x, void *y, size_t size) {
    uint8_t temp[size];

    memcpy(temp, x, size);
    memcpy(x, y, size);
    memcpy(y, temp, size);
}

/**
 * partition
 *  @base: the array/partition
 *  @low: lower index
 *  @high: higher index
 *  @size: data size
 *  @cmp: comparison function
 * 
 *  Divides an array into two partitions
 * 
 *  Returns the pivot index
 */
static size_t partition(void *base, size_t low, size_t high, size_t size, vector_cmp_fn cmp) {
    uint8_t *arr = (uint8_t*)base;
    void *pivot = arr + (high * size);
    size_t i = low;

    for (size_t j = low; j < high; j++) {
        vector_order_t order = cmp(arr + (j * size), pivot);

        if (order == VECTOR_ORDER_LT || order == VECTOR_ORDER_EQ) {
            swap(arr + (i * size), arr + (j * size), size);
            i++;
        }
    }

    swap(arr + (i * size), arr + (high * size), size);

    return i;
}

/**
 * quicksort
 *  @base: the base array/partition
 *  @low: lower index
 *  @high: higher index
 *  @size: data size
 *  @cmp: comparision function
 * 
 *  Recursively sorts an array/partition using the Quicksort algorithm
 */
static void quicksort(void *base, size_t low, size_t high, size_t size, vector_cmp_fn cmp) {
    if (low < high) {
        const size_t pivot = partition(base, low, high, size, cmp);

        if (pivot > 0) {
            quicksort(base, low, pivot - 1, size, cmp);
        }
        quicksort(base, pivot + 1, high, size, cmp);
    }
}


/**
 * vector_new
 *  @size: initial number of elements
 *  @data_size: size of each element in bytes
 *
 *  Returns a vector_result_t data type containing a new vector
 */
vector_result_t vector_new(size_t size, size_t data_size) {
    vector_result_t result = {0};

    if (size == 0) {
        result.status = VECTOR_ERR_ALLOCATE;
        SET_MSG(result, "Invalid vector size");

        return result;
    }

    // Allocate a new vector
    vector_t *vector = malloc(sizeof(vector_t));
    if (vector == NULL) {
        result.status = VECTOR_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for vector");
        
        return result;
    }

    // Initialize vector
    vector->size = 0;
    vector->capacity = size;
    vector->data_size = data_size;
    vector->elements = calloc(size, data_size);
    if (vector->elements == NULL) {
        free(vector);
        result.status = VECTOR_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for vector elements");

        return result;
    }

    result.status = VECTOR_OK;
    SET_MSG(result, "Vector successfully created");
    result.value.vector = vector;

    return result;
}

/**
 * vector_push
 *  @vector: a non-null vector
 *  @value: a generic value to add to the vector
 *
 *  Adds @value at the end of @vector
 *  
 *  Returns a vector_result_t data type containing the status
 */
vector_result_t vector_push(vector_t *vector, void *value) {
    vector_result_t result = {0};

    if (vector == NULL || value == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector or value");

        return result;
    }

    // Check whether vector has enough space available
    if (vector->size == vector->capacity) {
        result = vector_resize(vector);
        if (result.status != VECTOR_OK) {
            return result;
        }
    }

    // Calculate destination memory address
    uint8_t *destination_addr = (uint8_t*)vector->elements + (vector->size * vector->data_size);

    // Append @value to the data structure according to its data type
    if (vector->data_size == sizeof(int)) {
        *(int*)destination_addr = *(int*)value;
    } else if (vector->data_size == sizeof(long)) {
        *(long*)destination_addr = *(long*)value;
    } else if (vector->data_size == sizeof(double)) {
        *(double*)destination_addr = *(double*)value;
    } else if (vector->data_size == sizeof(float)) {
        *(float*)destination_addr = *(float*)value;
    } else {
        memcpy(destination_addr, value, vector->data_size);
    }

    // Increase elements count
    vector->size++;

    result.status = VECTOR_OK;
    SET_MSG(result, "Value successfully added");

    return result;
}

/**
 * vector_set
 *  @vector: a non-null vector
 *  @index: a non-negative integer representing the position to write into
 *  @value: a generic value to add to the vector
 *
 *  Writes @value at @index
 *
 *  Returns a vector_result_t data type
 */
vector_result_t vector_set(vector_t *vector, size_t index, void *value) {
    vector_result_t result = {0};

    if (vector == NULL || value == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector or value");

        return result;
    }

    if (index >= vector->size) {
        result.status = VECTOR_ERR_OVERFLOW;
        SET_MSG(result, "Index out of bounds");

        return result;
    }

    uint8_t *destination_addr = (uint8_t *)vector->elements + (index * vector->data_size);

    // Append @value to the data structure according to its data type
    if (vector->data_size == sizeof(int)) {
        *(int*)destination_addr = *(int*)value;
    } else if (vector->data_size == sizeof(long)) {
        *(long*)destination_addr = *(long*)value;
    } else if (vector->data_size == sizeof(double)) {
        *(double*)destination_addr = *(double*)value;
    } else if (vector->data_size == sizeof(float)) {
        *(float*)destination_addr = *(float*)value;
    } else {
        memcpy(destination_addr, value, vector->data_size);
    }

    result.status = VECTOR_OK;
    SET_MSG(result, "Value successfully set");

    return result;
}

/**
 * vector_get
 *  @vector: a non-null vector
 *  @index: a non-negative integer representing the position of an element
 *
 *  Returns a vector_result_t data type containing the element at position @index if available
 */
vector_result_t vector_get(vector_t *vector, size_t index) {
    vector_result_t result = {0};
    
    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector");
        
        return result;
    }

    if (index >= vector->size) {
        result.status = VECTOR_ERR_OVERFLOW;
        SET_MSG(result, "Index out of bounds");

        return result;
    }

    result.status = VECTOR_OK;
    SET_MSG(result, "Value successfully retrieved");
    result.value.element = (uint8_t *)vector->elements + (index * vector->data_size);

    return result;
}

/**
 * vector_sort
 *  @vector: a non-null vector
 *  @cmp: a user-defined comparison function returning vector_order_t
 * 
 *  Sorts @vector using Quicksort algorithm and the @cmp comparison function
 * 
 *  Returns a vecto_result_t data type
 */
vector_result_t vector_sort(vector_t *vector, vector_cmp_fn cmp) {
    vector_result_t result = {0};

    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector");

        return result;
    }

    if (cmp == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid comparison function");

        return result;
    }

    // The vector is already sorted
    if (vector->size <= 1) {
        result.status = VECTOR_OK;
        SET_MSG(result, "Vector successfully sorted");

        return result;
    }

    quicksort(vector->elements, 0, vector->size - 1, vector->data_size, cmp);

    result.status = VECTOR_OK;
    SET_MSG(result, "Vector successfully sorted");

    return result;
}

/**
 * vector_pop
 *  @vector: a non-null vector
 *
 *  Logically extract an element from the vector by following the LIFO policy.
 *  This method does NOT de-allocate memory
 *
 *  Returns a vector_result_t data type
 */
vector_result_t vector_pop(vector_t *vector) {
    vector_result_t result = {0};

    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector");
        
        return result;
    }

    if (vector->size == 0) {
        result.status = VECTOR_ERR_UNDERFLOW;
        SET_MSG(result, "Vector is empty");

        return result;
    }
    
    // Pop an element from the vector
    const size_t index = (vector->size - 1);
    vector_result_t popped_res = vector_get(vector, index);

    if (popped_res.status != VECTOR_OK) {
        return popped_res;
    }

    vector->size--;

    result.status = VECTOR_OK;
    SET_MSG(result, "Value successfully popped");
    result.value.element = popped_res.value.element;

    return result;
}

/**
 * vector_map
 *  @vector: a non-null vector
 *  @callback: callback function
 *  @env: optional captured environment
 *
 *  Transforms each element of @vector in place by applying @callback
 *
 *  Returns a vector_result_t data type
 */
vector_result_t vector_map(vector_t *vector, map_callback_fn callback, void *env) {
    vector_result_t result = {0};

    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector");

        return result;
    }

    if (callback == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid callback function");

        return result;
    }

    for (size_t idx = 0; idx < vector->size; idx++) {
        void *element = (uint8_t*)vector->elements + (idx * vector->data_size);
        callback(element, env);
    }

    result.status = VECTOR_OK;
    SET_MSG(result, "Vector successfully mapped");

    return result;
}

/**
 * vector_filter
 *  @vector: a non-null vector
 *  @callback: callback function
 *  @env: optional captured environment
 *
 *  Filters elements from @vector using @callback.
 *  Elements are shifted in place, vector size is updated.
 *
 *  Returns a vector_result_t data type
 */
vector_result_t vector_filter(vector_t *vector, vector_filter_fn callback, void *env) {
    vector_result_t result = {0};

    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector");

        return result;
    }

    if (callback == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid callback function");

        return result;
    }

    size_t write_idx = 0;
    for (size_t read_idx = 0; read_idx < vector->size; read_idx++) {
        void *element = (uint8_t*)vector->elements + (read_idx * vector->data_size);

        // Remove elements from @vector for which @callback returns zero
        // If @callback returns non-zero, element is kept
        if (callback(element, env)) {
            if (read_idx != write_idx) {
                void *dest = (uint8_t*)vector->elements + (write_idx * vector->data_size);
                memcpy(dest, element, vector->data_size);
            }
            write_idx++;
        }
    }

    // Update vector size
    vector->size = write_idx;

    result.status = VECTOR_OK;
    SET_MSG(result, "Vector successfully filtered");

    return result;
}

/**
 * vecto_reduce
 *  @vector: a non-null vector
 *  @accumulator: pointer to accumulator value
 *  @callback: callback function
 *  @env: optional captured environment
 *
 *  Reduces @vector to a single value by repeatedly applying @callback
 *  The @accumulator value should be initialized by the caller before invoking this function
 *
 *  Returns a vector_result_t data type
 */
vector_result_t vector_reduce(const vector_t *vector, void *accumulator, vector_reduce_fn callback, void *env) {
    vector_result_t result = {0};

    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector");

        return result;
    }

    if (accumulator == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid accumulator");

        return result;
    }

    if (callback == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid callback function");

        return result;
    }

    for (size_t idx = 0; idx < vector->size; idx++) {
        const void *element = (uint8_t*)vector->elements + (idx * vector->data_size);
        callback(accumulator, element, env);
    }

    result.status = VECTOR_OK;
    SET_MSG(result, "Vector successfully reduced");

    return result;
}

/**
 * vector_clear
 *  @vector: a non-null vector
 *
 *  Resets the vector to an empty state without de-allocating memory
 *
 *  Returns a vector_result_t data type
 */
vector_result_t vector_clear(vector_t *vector) {
    vector_result_t result = {0};

    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector");

        return result;
    }

    vector->size = 0;

    result.status = VECTOR_OK;
    SET_MSG(result, "Vector successfully cleared");

    return result;
}

/**
 * vector_destroy
 *  @vector: a non-null vector
 *
 *  Deletes the vector and all its elements from the memory
 *
 *  Returns a vector_result_t data type
 */
vector_result_t vector_destroy(vector_t *vector) {
    vector_result_t result = {0};

    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        SET_MSG(result, "Invalid vector");

        return result;
    }

    free(vector->elements);
    free(vector);

    result.status = VECTOR_OK;
    SET_MSG(result, "Vector successfully deleted");

    return result;
}
