#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

// Internal method to increase vector size
static VectorResult vector_resize(Vector *vector);

/**
 * vector_new
 *  @size: initial number of elements
 *  @data_size: size of each element in bytes
 *
 *  Returns a VectorResult data type containing a new vector
 */
VectorResult vector_new(size_t size, size_t data_size) {
    VectorResult result = {0};

    // Allocate a new vector
    Vector *vector = malloc(sizeof(Vector));
    if (vector == NULL) {
        result.status = VECTOR_ERR_ALLOCATE;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Failed to allocate memory for vector");
        
        return result;
    }

    // Initialize vector
    vector->count = 0;
    vector->capacity = size;
    vector->data_size = data_size;
    vector->elements = calloc(size, data_size);
    if (vector->elements == NULL) {
        result.status = VECTOR_ERR_ALLOCATE;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Failed to allocate memory for vector elements");

        return result;
    }

    result.status = VECTOR_OK;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Vector successfully created");
    result.value.vector = vector;

    return result;
}

/**
 * vector_resize
 *  @vector: a non-null vector
 *
 *  Increases the size of @vector
 *
 *  Returns a VectorResult data type containing the status
 */
VectorResult vector_resize(Vector *vector) {
    VectorResult result = {0};

    size_t old_capacity = vector->capacity;
    vector->capacity = (old_capacity > 0 ? ((old_capacity * 3) / 2) : 1);

    // Check for stack overflow errors
    if (vector->capacity > SIZE_MAX / vector->data_size) {
        result.status = VECTOR_ERR_OVERFLOW;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Exceeded maximum size while resizing vector");

        return result;
    }

    void *new_elements = realloc(vector->elements, (vector->capacity * vector->data_size));
    if (new_elements == NULL) {
        result.status = VECTOR_ERR_ALLOCATE;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Failed to reallocate memory for vector");

        return result;
    }

    vector->elements = new_elements;

    result.status = VECTOR_OK;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Vector successfully resized");

    return result;
}

/**
 * vector_push
 *  @vector: a non-null vector
 *  @value: a generic value to add to the vector
 *
 *  Adds @value at the end of @vector
 *  
 *  Returns a VectorResult data type containing the status
 */
VectorResult vector_push(Vector *vector, void *value) {
    VectorResult result = {0};

    if (vector == NULL || value == NULL) {
        result.status = VECTOR_ERR_INVALID;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Invalid vector or value");

        return result;
    }

    // Check whether vector has enough space available
    if (vector->capacity == vector->count) {
        result = vector_resize(vector);
        if (result.status != VECTOR_OK) {
            return result;
        }
    }

    // Calculate destination memory address
    uint8_t *destination_addr = (uint8_t*)vector->elements + (vector->count * vector->data_size);

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
    vector->count++;

    result.status = VECTOR_OK;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Value successfully added");

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
 *  Returns a VectorResult data type
 */
VectorResult vector_set(Vector *vector, size_t index, void *value) {
    VectorResult result = {0};

    if (vector == NULL || value == NULL) {
        result.status = VECTOR_ERR_INVALID;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Invalid vector or value");

        return result;
    }

    if (index >= vector->count) {
        result.status = VECTOR_ERR_OVERFLOW;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Index out of bounds");

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
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Value successfully set");

    return result;
}

/**
 * vector_get
 *  @vector: a non-null vector
 *  @index: a non-negative integer representing the position of an element
 *
 *  Returns a VectorResult data type containing the element at position @index if present
 */
VectorResult vector_get(Vector *vector, size_t index) {
    VectorResult result = {0};
    
    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Invalid vector");
        
        return result;
    }

    if (index >= vector->count) {
        result.status = VECTOR_ERR_OVERFLOW;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Index out of bounds");

        return result;
    }

    result.status = VECTOR_OK;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Value successfully retrieved");
    result.value.element = (uint8_t *)vector->elements + (index * vector->data_size);

    return result;
}

/**
 * vector_pop
 *  @vector: a non-null vector
 *
 *  Logically extract an element from the vector by following the LIFO policy.
 *  This method does NOT de-allocate memory
 *
 *  Returns a VectorResult data type
 */
VectorResult vector_pop(Vector *vector) {
    VectorResult result = {0};

    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Invalid vector");
        
        return result;
    }

    if (vector->count == 0) {
        result.status = VECTOR_ERR_UNDERFLOW;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Vector is empty");

        return result;
    }
    
    // Pop an element from the vector
    const size_t index = (vector->count - 1);
    VectorResult popped_res = vector_get(vector, index);

    if (popped_res.status != VECTOR_OK) {
        return popped_res;
    }

    vector->count--;

    result.status = VECTOR_OK;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Value successfully popped");
    result.value.element = popped_res.value.element;

    return result;
}

/**
 * vector_clear
 *  @vector: a non-null vector
 *
 *  Resets the vector to an empty state without de-allocating memory
 *
 *  Returns a VectorResult data type
 */
VectorResult vector_clear(Vector *vector) {
    VectorResult result = {0};

    if (vector == NULL) {
        result.status = VECTOR_ERR_INVALID;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Invalid vector");

        return result;
    }

    vector->count = 0;

    result.status = VECTOR_OK;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Vector successfully cleared");

    return result;
}

/**
 * vector_free
 *  @vector: a non-null vector
 *
 *  Deletes the vector and all its elements from the memory
 *
 *  Returns a VectorResult data type
 */
VectorResult vector_free(Vector *vector) {
    VectorResult result = {0};

    if (vector != NULL) {
        free(vector->elements);
        free(vector);
    }

    result.status = VECTOR_OK;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Vector successfully deleted");

    return result;
}
