# Vector Technical Details
In this document you can find a quick overview of the technical
aspects (internal design, memory layout, etc.) of the `Vector` data structure. 

`Vector` is a dynamic array with generic data type support; this means that you can store
any kind of homogenous value on this data structure. Resizing is performed automatically
by increasing the capacity by 1.5 times when the array becomes full. Internally, this 
data structure is represented by the following structure:

```c
typedef struct {
    size_t size;
    size_t capacity;
    size_t data_size;
    void *elements;
} vector_t;
```

where the `elements` variable represents the actual dynamic and generic array, the
`data_size` variable indicates the size (in bytes) of the data type while the `size`
and the `capacity` represent the number of store elements and the total size of
the structure, respectively. The dynamic array copies the values upon insertion,
thus **it owns the data** and is therefore responsible for its allocation and its
deletion.

At the time being, `Vector` supports the following methods:

- `vector_result_t vector_new(size, data_size)`: create a new vector;  
- `vector_result_t vector_push(vector, value)`: add a new value to the vector;  
- `vector_result_t vector_set(vector, index, value)`: update the value of a given index if it exists;  
- `vector_result_t vector_get(vector, index)`: return the value indexed by `index` if it exists;  
- `map_result_t vector_sort(map, cmp)`: sort array using `cmp` function;  
- `vector_result_t vector_pop(vector)`: pop last element from the vector following the LIFO policy;  
- `vector_result_t vector_map(vector, callback, env)`: apply `callback` function to vector (in-place);  
- `vector_result_t vector_filter(vector, callback, env)`: filter vector using `callback` (in-place);  
- `vector_result_t vector_reduce(vector, accumulator, callback, env)`: fold/reduce vector using `callback`;  
- `vector_result_t vector_clear(vector)`: logically reset the vector. That is, new pushes will overwrite the memory;  
- `vector_result_t vector_destroy(vector)`: delete the vector;  
- `size_t vector_size(vector)`: return vector size (i.e., the number of elements);  
- `size_t vector_capacity(vector)`: return vector capacity (i.e., vector total size).

As you can see by the previous function signatures, most methods that operate
on the `Vector` data type return a custom type called `vector_result_t` which is
defined as follows:

```c
typedef enum {
    VECTOR_OK = 0x0,
    VECTOR_ERR_ALLOCATE,
    VECTOR_ERR_OVERFLOW,
    VECTOR_ERR_UNDERFLOW,
    VECTOR_ERR_INVALID
} vector_status_t;

typedef struct {
    vector_status_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        vector_t *vector;
        void *element;
    } value;
} vector_result_t;
```

Each method that returns such type indicates whether the operation was successful or not
by setting the `status` field and by providing a descriptive message on the `message`
field. If the operation was successful (that is, `status == VECTOR_OK`), you can either
move on with the rest of the program or read the returned value from the sum data type. Of course, you can choose to 
ignore the return value (if you're brave enough :D) as illustrated in the first part of the README.

## Functional methods
`Vector` provides three functional methods called `map`, `filter` and `reduce` which allow the caller to apply a computation to the vector,
filter the vector according to a function and fold the vector to a single value according to a custom function, respectively.

The caller is responsible to define a custom `callback` function that satisfy the following constraints:

```c
typedef void (*map_callback_fn)(void *element, void *env);
typedef int (*vector_filter_fn)(const void *element, void *env);
typedef void (*vector_reduce_fn)(void *accumulator, const void *element, void *env);
```

In particular, you should be aware of the following design choices:

- The `vector_reduce` callback method requires the caller to initialize an _"accumulator"_ variable before calling this method;  
- The `vector_filter` callback method is expected to return non-zero to keep the element and zero to filter it out.  

The documentation for the `vector_sort(map, cmp)` method can be found in [the following document](/docs/sort.md). 
