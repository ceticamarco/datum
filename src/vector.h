#ifndef VECTOR_H
#define VECTOR_H

#define RESULT_MSG_SIZE 64

#include <stdint.h>
#include <stddef.h>

typedef enum {
    VECTOR_OK = 0x0,
    VECTOR_ERR_ALLOCATE,
    VECTOR_ERR_OVERFLOW,
    VECTOR_ERR_UNDERFLOW,
    VECTOR_ERR_INVALID
} vector_status_t;

typedef struct {
    size_t count;
    size_t capacity;
    size_t data_size;
    void *elements;
} vector_t;

typedef struct {
    vector_status_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        vector_t *vector;
        void *element;
    } value;
} vector_result_t;

#ifdef __cplusplus
extern "C" {
#endif

// public APIs
vector_result_t vector_new(size_t size, size_t data_size);
vector_result_t vector_push(vector_t *vector, void *value);
vector_result_t vector_set(vector_t *vector, size_t index, void *value);
vector_result_t vector_get(vector_t *vector, size_t index);
vector_result_t vector_pop(vector_t *vector);
vector_result_t vector_clear(vector_t *vector);
vector_result_t vector_free(vector_t *vector);

// Inline methods
static inline size_t vector_size(const vector_t *vector) {
    return vector ? vector->count : 0;
}

static inline size_t vector_capacity(const vector_t *vector) {
    return vector ? vector->capacity : 0;
}

#ifdef __cplusplus
}
#endif

#endif
