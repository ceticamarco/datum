#ifndef MAP_H
#define MAP_H

#define RESULT_MSG_SIZE 64

// Initial capacity and load factor threshold
#define INITIAL_CAP 4
#define LOAD_FACTOR_THRESHOLD 0.75

// FNV-1a constants
#define FNV_OFFSET_BASIS_64 0xCBF29CE484222325
#define FNV_PRIME_64 0x00000100000001B3

#include <stdint.h>
#include <stddef.h>

typedef enum {
    MAP_OK = 0x0,
    MAP_ERR_ALLOCATE,
    MAP_ERR_INVALID,
    MAP_ERR_NOT_FOUND
} map_status_t;

typedef enum {
    ENTRY_EMPTY = 0x0,
    ENTRY_OCCUPIED,
    ENTRY_DELETED
} element_state_t;

typedef struct {
    char *key;
    void *value;
    element_state_t state;
} map_element_t;

typedef struct {
    map_element_t *elements;
    size_t capacity;
    size_t size;
    size_t tombstone_count;
} map_t;

typedef struct {
    map_status_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        map_t *map;
        void *element;
    } value;
} map_result_t;

#ifdef __cplusplus
extern "C" {
#endif

map_result_t map_new();
map_result_t map_add(map_t *map, const char *key, void *value);
map_result_t map_get(const map_t *map, const char *key);
map_result_t map_remove(map_t *map, const char *key);
map_result_t map_clear(map_t *map);
map_result_t map_destroy(map_t *map);

// Inline methods
static inline size_t map_size(const map_t *map) {
    return map ? map->size : 0;
}

static inline size_t map_capacity(const map_t *map) {
    return map ? map->capacity : 0;
}

#ifdef __cplusplus
}
#endif

#endif
