#define SET_MSG(result, msg) \
    do { \
        snprintf((char *)(result).message, RESULT_MSG_SIZE, "%s", (const char *)msg); \
    } while (0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

// Internal methods
static uint64_t hash_key(const char *key);
static size_t map_insert_index(const map_t *map, const char *key);
static size_t map_find_index(const map_t *map, const char *key);
static map_result_t map_resize(map_t *map);

/**
 * hash_key
 *  @key: The input string for the hash function
 *
 *  Returns the digest of @key using the Fowler-Noll-Vo hashing algorithm
 */
uint64_t hash_key(const char *key) {
    uint64_t hash = FNV_OFFSET_BASIS_64;

    while (*key) {
        hash ^= (uint64_t)*(key++);
        hash *= FNV_PRIME_64;
    }

    return hash;
}

/**
 * map_new
 *
 * Returns a map_result_t data type containing a new hash map
 */
map_result_t map_new() {
    map_result_t result = {0};

    map_t *map = malloc(sizeof(map_t));
    if (map == NULL) {
        result.status = MAP_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for map");

        return result;
    }

    map->elements = calloc(INITIAL_CAP, sizeof(map_element_t));
    if (map->elements == NULL) {
        free(map);
        result.status = MAP_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for map elements");

        return result;
    }

    // Initialize map
    map->capacity = INITIAL_CAP;
    map->size = 0;
    map->tombstone_count = 0;

    result.status = MAP_OK;
    SET_MSG(result, "Map successfully created");
    result.value.map = map;

    return result;
}

/**
 * map_insert_index
 *  @map: a non-null map
 *  @key: a string representing the key to find 
 *  
 *  Finds next available slot for insertion
 *
 *  Returns the index of available slot
 */
size_t map_insert_index(const map_t *map, const char *key) {
    const uint64_t key_digest = hash_key(key);
    size_t idx = key_digest % map->capacity;

    while (map->elements[idx].state == ENTRY_OCCUPIED) {
        if (strcmp(map->elements[idx].key, key) == 0) {
            // In this case the key already exists, thus we replace it
            return idx;
        }

        idx = (idx + 1) % map->capacity;
    }

    return idx;
}

/**
 * @map: a non-null map
 *
 * Increases the size of @map
 *
 * Returns a a map_result_t data type containing the status
 */
map_result_t map_resize(map_t *map) {
    map_result_t result = {0};

    const size_t old_capacity = map->capacity;
    map_element_t *old_elements = map->elements;

    map->capacity *= 2;
    map->elements = calloc(map->capacity, sizeof(map_element_t));
    if (map->elements == NULL) {
        // Restore old parameters if resize failed
        map->capacity = old_capacity;
        map->elements = old_elements;

        result.status = MAP_ERR_ALLOCATE;
        SET_MSG(result, "Failed to reallocate memory for map");

        return result;
    }

    map->size = 0;
    map->tombstone_count = 0;

    // Rehash all existing elements
    for (size_t idx = 0; idx < old_capacity; idx++) {
        if (old_elements[idx].state == ENTRY_OCCUPIED) {
            size_t new_idx = map_insert_index(map, old_elements[idx].key);
            map->elements[new_idx] = old_elements[idx];
            map->size++;
        } else if (old_elements[idx].state == ENTRY_DELETED) {
            free(old_elements[idx].key);
        }
    }

    free(old_elements);

    result.status = MAP_OK;
    SET_MSG(result, "Map successfully resized");

    return result;
}

/**
 * map_add
 *  @map: a non-null map
 *  @key: a string representing the index key
 *  @value: a generic value to add to the map
 *
 *  Adds (@key, @value) to @map
 *
 *  Returns a map_result_t data type containing the status 
 */
map_result_t map_add(map_t *map, const char *key, void *value) {
    map_result_t result = {0};

    if (map == NULL || key == NULL) {
        result.status = MAP_ERR_INVALID;
        SET_MSG(result, "Invalid map or key");

        return result;
    }

    // Check whether there's enough space available
    const double load_factor = (double)(map->size + map->tombstone_count) / map->capacity;
    if (load_factor > LOAD_FACTOR_THRESHOLD) {
        result = map_resize(map);
        if (result.status != MAP_OK) {
            return result;
        }
    }

    // Find next available slot for insertion
    const size_t idx = map_insert_index(map, key);

    // If slot is occupied, it means that the key already exists.
    // Therefore we can update it
    if (map->elements[idx].state == ENTRY_OCCUPIED) {
        map->elements[idx].value = value;

        result.status = MAP_OK;
        SET_MSG(result, "Element successfully updated");

        return result;
    }

    // Otherwise, the key doesn't exist. Therefore we need to allocate a new key
    map->elements[idx].key = malloc(strlen(key) + 1);
    if (map->elements[idx].key == NULL) {
        result.status = MAP_ERR_ALLOCATE;
        SET_MSG(result, "Failed to allocate memory for map key");

        return result;
    }

    strcpy(map->elements[idx].key, key);
    map->elements[idx].value = value;
    map->elements[idx].state = ENTRY_OCCUPIED;
    map->size++;

    result.status = MAP_OK;
    SET_MSG(result, "Element successfully added");

    return result;
}

/**
 * map_find_index
 *  @map: a non-null map
 *  @key: a string representing the index key to find
 *
 *  Finds the index where a key is located using linear probing to handle collisions
 *
 *  Returns the index of the key if it is found
 */
size_t map_find_index(const map_t *map, const char *key) {
    const uint64_t key_digest = hash_key(key);
    size_t idx = key_digest % map->capacity;

    while (map->elements[idx].state != ENTRY_EMPTY) {
        if ((map->elements[idx].state == ENTRY_OCCUPIED) && 
            (strcmp(map->elements[idx].key, key) == 0)) {
            return idx;
        }
        idx = (idx + 1) % map->capacity;
    }

    return idx;
}

/**
 * map_get
 *  @map: a non-null map
 *  @key: a string representing the index key
 *
 *  Returns a map_result_t data type containing the element indexed by @key if available
 */
map_result_t map_get(const map_t *map, const char *key) {
    map_result_t result = {0};

    if (map == NULL || key == NULL) {
        result.status = MAP_ERR_INVALID;
        SET_MSG(result, "Invalid map or key");

        return result;
    }

    // Retrieve key index
    const size_t idx = map_find_index(map, key);

    // If slot status is 'occupied' then the key exists
    if (map->elements[idx].state == ENTRY_OCCUPIED) {
        result.status = MAP_OK;
        SET_MSG(result, "Value successfully retrieved");
        result.value.element = map->elements[idx].value;

        return result;
    }

    result.status = MAP_ERR_NOT_FOUND;
    SET_MSG(result, "Element not found");

    return result;
}

/**
 * map_remove
 *  @map: a non-null map
 *  @key: a string representing the index key
 *
 *  Removes an element indexed by @key from @map
 *
 *  Returns a map_result_t data type
 */
map_result_t map_remove(map_t *map, const char *key) {
    map_result_t result = {0};

    if (map == NULL) {
        result.status = MAP_ERR_INVALID;
        SET_MSG(result, "Invalid map");

        return result;
    }

    const size_t idx = map_find_index(map, key);

    if (map->elements[idx].state != ENTRY_OCCUPIED) {
        result.status = MAP_ERR_INVALID;
        SET_MSG(result, "Cannot delete this element");

        return result;
    }

    // Remove element key
    free(map->elements[idx].key);

    // Remove element properties
    map->elements[idx].key = NULL;
    map->elements[idx].value = NULL;
    map->elements[idx].state = ENTRY_DELETED;

    // Decrease map size and increase its tombstone count
    map->size--;
    map->tombstone_count++;

    result.status = MAP_OK;
    SET_MSG(result, "Key successfully deleted");

    return result;
}

/**
 * map_clear
 *  @map: a non-null map
 *
 *  Resets the map to an empty state
 *
 *  Returns a map_result_t data type
 */
map_result_t map_clear(map_t *map) {
    map_result_t result = {0};

    if (map == NULL) {
        result.status = MAP_ERR_INVALID;
        SET_MSG(result, "Invalid map");

        return result;
    }

    for (size_t idx = 0; idx < map->capacity; idx++) {
        if (map->elements[idx].state == ENTRY_OCCUPIED ||
            map->elements[idx].state == ENTRY_DELETED) {
            free(map->elements[idx].key);
            map->elements[idx].key = NULL;
            map->elements[idx].value = NULL;
        }

        map->elements[idx].state = ENTRY_EMPTY;
    }

    // Resets map size and tombstone count
    map->size = 0;
    map->tombstone_count = 0;

    result.status = MAP_OK;
    SET_MSG(result, "Map successfully cleared");

    return result;
}

/**
 * map_destroy
 *  @map: a non-null map
 *
 *  Deletes the map and all its elements from the memory
 *
 *  Returns a map_result_t data type
 */
map_result_t map_destroy(map_t *map) {
    map_result_t result = {0};

    if (map == NULL) {
        result.status = MAP_ERR_INVALID;
        SET_MSG(result, "Invalid map");

        return result;
    }

    for (size_t idx = 0; idx < map->capacity; idx++) {
        if (map->elements[idx].state == ENTRY_OCCUPIED ||
            map->elements[idx].state == ENTRY_DELETED) {
            free(map->elements[idx].key);
        }
    }

    free(map->elements);
    free(map);

    result.status = MAP_OK;
    SET_MSG(result, "Map successfully deleted");

    return result;
}
