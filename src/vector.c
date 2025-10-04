#include <stdio.h> // for snprintf
#include <stdlib.h> // for malloc, realloc and free
#include <string.h> // for memcpy

#include "vector.h"

// Internal method to increase vector size
static Result vector_resize(Vector *vector);

/**
 * vector_new
 *  @size: initial number of elements
 *  @data_size: size of each element in bytes
 *
 *  Returns a Result data type containing a new vector
 */
Result vector_new(size_t size, size_t data_size) {
    Result result = {0};

    // Allocate a new vector
    Vector *vector = malloc(sizeof(Vector));
    if (vector == NULL) {
        result.status = 1;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Failed to allocate memory for vector");
        
        return result;
    }

    // Initialize vector
    vector->count = 0;
    vector->capacity = size;
    vector->data_size = data_size;
    vector->elements = calloc(size, data_size);
    if (vector->elements == NULL) {
        result.status = 1;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Failed to allocate memory for vector elements");

        return result;
    }

    result.status = 0;
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
 *  Returns a Result data type containing the status
 */
Result vector_resize(Vector *vector) {
    Result result = {0};

    size_t old_capacity = vector->capacity;
    vector->capacity = (old_capacity > 0 ? ((old_capacity * 3) / 2) : 1);

    // Check for stack overflow errors
    if (vector->capacity > SIZE_MAX / vector->data_size) {
        result.status = 1;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Exceeded maximum size while resizing vector");

        return result;
    }

    void *new_elements = realloc(vector->elements, (vector->capacity * vector->data_size));
    if (new_elements == NULL) {
        result.status = 1;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Failed to reallocate memory for vector");

        return result;
    }

    vector->elements = new_elements;

    result.status = 0;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Vector successfully resized");

    return result;
}

/**
 * vector_push
 *  @vector: a non-null vector
 *  @value: a generic value to add on the vector
 *
 *  Adds @value at the end of @vector
 *  
 *  Returns a Result data type containing the status
 */
Result vector_push(Vector *vector, void *value) {
    Result result = {0};

    if (vector == NULL || value == NULL) {
        result.status = 1;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Invalid vector or value");

        return result;
    }

    // Check whether vector has enough space available
    if (vector->capacity == vector->count) {
        result = vector_resize(vector);
        if (result.status != 0) {
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

    result.status = 0;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Value successfully added");

    return result;
}

/**
 * vector_get
 *  @vector: a non-null vector
 *  @index: a non-negative integer representing the position of an element
 *
 *  Returns a Result data type containing the element at position @index if present
 */
Result vector_get(Vector *vector, size_t index) {
    Result result = {0};
    
    if (vector == NULL) {
        result.status = 1;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Invalid vector");
        
        return result;
    }

    if (index >= vector->count) {
        result.status = 1;
        snprintf((char *)result.message, RESULT_MSG_SIZE, "Index out of bounds");

        return result;
    }

    result.status = 0;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Value successfully retrieved");
    result.value.element = (uint8_t*)vector->elements + (index * vector->data_size);

    return result;
}


/**
 * vector_free
 *  @vector: a non-null vector
 *
 *  Deletes the vector and all its elements from the memory
 *
 *  Returns a Result data type
 */
Result vector_free(Vector *vector) {
    Result result = {0};

    if (vector != NULL) {
        free(vector->elements);
        free(vector);
    }

    result.status = 0;
    snprintf((char *)result.message, RESULT_MSG_SIZE, "Vector successfully deleted");

    return result;
}
