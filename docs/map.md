# Map Technical Details
In this document you can find a quick overview of the technical
aspects (internal design, memory layout, etc.) of the `Map` data structure. 

`Map` is an hash table that uses open addressing with linear probing for collision
resolution and the [FNV-1a algorithm](https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function) as its hashing function. Resizing is performed
automatically by doubling the capacity when the load factor exceeds 75%. Internally,
this data structure is represented by the following two structures:

```c
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
```

where the `key` variable represent a string used to index the `value`. The `state`, instead, indicates whether the entry is empty, occupied or deleted and is primarily used
by the garbage collector for internal memory management. An array of `map_element_t`,
with the variables indicating the *capacity*, the *current size* and
the *tombstone count* (that is, the number of delete entries), form a `map_t` data type.

The keys are **copied** by the hashmap; this means that it **owns** them and is therefore
responsible for managing their memory. Values, on the other hand, 
**are stored as pointers**. This means that the hashmap **does NOT own them** and that
the caller is responsible for managing their memory; this includes: allocate
enough memory for them, ensure that the pointers remain valid for their whole lifecycle
on the map, delete old values when updating a key and, if the values were heap-allocated,
free them before removing the keys or destroying the map.

The `Map` data structure supports the following methods:

- `map_result_t map_new()`: initialize a new map;  
- `map_result_t map_add(map, key, value)`: add a `(key, value)` pair to the map;  
- `map_result_t map_get(map, key)`: retrieve a values indexed by `key` if it exists;  
- `map_result_t map_remove(map, key)`: remove a key from the map if it exists;  
- `map_result_t map_clear(map)`: reset the map state;  
- `map_result_t map_destroy(map)`: delete the map;  
- `size_t map_size(map)`: returns map size (i.e., the number of elements);  
- `size_t map_capacity(map)`: returns map capacity (i.e., map total size).

As you can see by the previous function signatures, most methods that operate
on the `Map` data type return a custom type called `map_result_t` which is
defined as follows:

```c
typedef enum {
    MAP_OK = 0x0,
    MAP_ERR_ALLOCATE,
    MAP_ERR_OVERFLOW,
    MAP_ERR_INVALID,
    MAP_ERR_NOT_FOUND
} map_status_t;

typedef struct {
    map_status_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        map_t *map;
        void *element;
    } value;
} map_result_t;
```

Each method that returns such type indicates whether the operation was successful or not by setting
the `status` field and by providing a descriptive message on the `message` field. If the operation was
successful (that is, `status == MAP_OK`), you can either move on with the rest of the program or read
the returned value from the sum data type. Of course, you can choose to ignore the return value (if you're brave enough :D) as illustrated
in the first part of the README.