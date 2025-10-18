#ifndef VECTOR_H
#define VECTOR_H

#define RESULT_MSG_SIZE 64

#include <stdint.h>
#include <stddef.h>

// Vector data type
typedef struct {
    size_t count;
    size_t capacity;
    size_t data_size;
    void *elements;
} Vector;

// Result status codes
typedef enum {
    VECTOR_OK = 0x0,
    VECTOR_ERR_ALLOCATE,
    VECTOR_ERR_OVERFLOW,
    VECTOR_ERR_UNDERFLOW,
    VECTOR_ERR_INVALID
} VectorStatus;

// Wrapper data type for vector APIs
typedef struct {
    VectorStatus status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        Vector *vector;
        void *element;
    } value;
} VectorResult;

#ifdef __cplusplus
extern "C" {
#endif

// public APIs
VectorResult vector_new(size_t size, size_t data_size);
VectorResult vector_push(Vector *vector, void *value);
VectorResult vector_set(Vector *vector, size_t index, void *value);
VectorResult vector_get(Vector *vector, size_t index);
VectorResult vector_pop(Vector *vector);
VectorResult vector_clear(Vector *vector);
VectorResult vector_free(Vector *vector);

// Inline methods
static inline size_t vector_size(const Vector *vector) {
    return vector ? vector->count : 0;
}

static inline size_t vector_capacity(const Vector *vector) {
    return vector ? vector->capacity : 0;
}

#ifdef __cplusplus
}
#endif

#endif
