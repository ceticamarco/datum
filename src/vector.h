#ifndef VECTOR_H
#define VECTOR_H

#define RESULT_MSG_SIZE 64

#include <stdint.h> // for uint8_t
#include <stddef.h> // for size_t

// Vector data type
typedef struct {
    size_t count;
    size_t capacity;
    size_t data_size;
    void *elements;
} Vector;

// Result data type
typedef struct {
    uint8_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        Vector *vector;
        void *element;
    } value;
} Result;

#ifdef __cplusplus
extern "C" {
#endif

Result vector_new(size_t size, size_t data_size);
Result vector_push(Vector *vector, void *value);
Result vector_get(Vector *vector, size_t index);
Result vector_free(Vector *vector);

#ifdef __cplusplus
}
#endif

#endif
