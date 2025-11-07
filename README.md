<div align="center">
    <h1>Datum</h1>
    <h6><i>Collection of dynamic and generic data structures.</i></h6>

[![](https://github.com/ceticamarco/datum/actions/workflows/datum.yml/badge.svg)](https://github.com/ceticamarco/datum/actions/workflows/datum.yml)
</div>

Datum is a collection of dynamic and generic data structures implemented from scratch in C with no external dependencies beyond
the standard library. It currently features:

- **Vector**: a growable, contiguous array of homogenous generic data types;  
- **Map**: an associative array that handles generic heterogenous data types;  

## Usage
At its simplest, you can use this library as follows:

### `Vector`'s usage

```c
#include <stdio.h>
#include "src/vector.h"

/*
 * Compile with: gcc main.c src/vector.c
 * Output: First element: 5
 *         Head of vector 6, size is now: 1 
 */ 

int main(void) {
    // Create an integer vector of initial capacity equal to 5
    vector_t *vec = vector_new(5, sizeof(int)).value.vector;

    // Add two numbers
    int val = 5;
    vector_push(vec, &val);
    // Equivalent as above
    vector_push(vec, &(int){6});

    // Print 1st element
    const int first = *(int*)vector_get(vec, 0).value.element;
    printf("First element: %d\n", first);

    // Pop second element using LIFO policy
    const int head = *(int*)vector_pop(vec).value.element;
    printf("Head of vector %d, size is now: %zu\n", head, vector_size(vec));

    // Remove vector from memory
    vector_destroy(vec);
    
    return 0;
}
```

### `Map`'s usage

```c
#include <stdio.h>
#include "src/map.h"

typedef struct {
    char name[256];
    char surname[256];
    short age;
} Person;

/*
 * Compile with: gcc main.c src/map.c
 * Output: Name: Bob, Surname: Smith, Age: 34
 */
int main(void) {
    // Create a new map
    map_t *map = map_new().value.map;

    // Add a key to the map
    const Person bob = { .name = "Bob", .surname = "Smith", .age = 34 };
    map_add(map, "bob", (void*)&bob);

    // Retrieve 'Bob' and check if it exists
    map_result_t bob_res = map_get(map, "bob");
    if (bob_res.status == MAP_ERR_NOT_FOUND) {
        puts("This key does not exist.");
    } else {
        const Person *ret = (const Person*)bob_res.value.element;
        printf("Name: %s, Surname: %s, Age: %d\n", 
            ret->name, 
            ret->surname, 
            ret->age
        );
    }

    // Remove map from memory
    map_destroy(map);

    return 0;
}
```

For a more exhaustive example, refer to the `usage.c` file. There, you will find a program with proper error management
and a sample usage for every available method. To run it, first issue the following command:

```sh
$ make clean all
```

This will compile the library as well as the `usage.c` file and the unit tests. After that, you can run it by typing `./usage`.

## Technical Details
In this section, you can find a quick overview of the technical aspects (internal design, memory layout, etc.) of this library as well as an
overview about the design choices behind Datum. While both structures use `void*` to represent values, the way they manage memory is orthogonally different
from one another. Let's start with the `Map` data type.

### Map
`Map` is an hash table implementation that uses open addressing with linear probing for collision resolution and the 
[FNV-1a algorithm](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function) as its hashing function. Resizing is performed automatically
by doubling the capacity when load factor exceeds 75%. Internally, this data structure is represented
by the following structures:

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
where the `key` represent a string used to index the `value`. The state, instead, indicates
whether the entry is empty, occupied or deleted and is primarily used by the garbage collector
for internal memory management. An array of `map_element_t` as well as variables indicating
the capacity, the current size and the tombstone count (that is, the number of deleted entries)
forms a `map_t` data type.

The keys are **copied** by the hashmap. This means that the hashmap **owns** them and is responsible
to manage their memory. Values, on the other hand, **are stored as pointers**. This means that the hashmap **does NOT own them** and that the caller is responsible
for managing their memory; this includes: allocate enough memory for them, ensure that the pointers remain valid for their whole lifecycle on the map, 
delete old values when updating a key and, if the values were heap-allocated, free them before removing them or before destroying the map.

The `Map` data structures supports the following methods:

- `map_result_t map_new()`: initialize a new map;  
- `map_result_t map_add(map, key, value)`: add a `(key, value)` pair to the map;  
- `map_result_t map_get(map, key)`: retrieve a values indexed by `key` if it exists;  
- `map_result_t map_remove(map, key)`: remove a key from the map if it exists;  
- `map_result_t map_clear(map)`: reset the map state;  
- `map_result_t map_destroy(map)`: delete the map;  
- `size_t map_size(map)`: returns map size (i.e., the number of elements);  
- `size_t map_capacity(map)`: returns map capacity (i.e., map total size).

As you can see, most methods that operates on the `Map` data type return a custom type called `map_result_t` which is defined as follows:

```c
typedef enum {
    MAP_OK = 0x0,
    MAP_ERR_ALLOCATE,
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

Each method that returns a `map_result_t` indicates whether the operation was successful or not by setting the `status` field and by providing a descriptive message on the `message` field. 
If the operation was successful (that is, `status == MAP_OK`), you can either move on with the flow
of the program or read the returned
value from the sum data type. Of course, 
you can choose to ignore the return value (if you're brave enough :D), as illustrated in the first example of this document.

### Vector
`Vector` is a dynamic array with generic data type support, this means that you can store any kind of homogenous value on this data structure. As in the `Map`'s case,
resizing is performed automatically by increasing the capacity by 1.5 times when the array is full. Internally, this data structure is represented as follows:

```c
typedef struct {
    size_t count;
    size_t capacity;
    size_t data_size;
    void *elements;
} vector_t;
```

where the `elements` represents the actual dynamic and generic array, the `data_size`
variable indicates the size (in bytes) of the data type while the count and
the capacity represent the number of stored elements and the total
size of the structure, respectively. The dynamic array copies the values upon 
insertion, thus **it owns the data** and is therefore responsible for their 
allocation and their deletion.

The dynamic array copies the values upon insertion, thus it is responsible
for their allocation and their deletion.

The `Vector` data structure supports the following methods:

- `vector_result_t vector_new(size, data_size)`: create a new vector;  
- `vector_result_t vector_push(vector, value)`: add a new value to the vector;  
- `vector_result_t vector_set(vector, index, value)`: update the value of a given index if it exists;  
- `vector_result_t vector_get(vector, index)`: return the value indexed by `index` if it exists;  
- `map_result_t vector_sort(map, cmp)`: sort array using `cmp` function;  
- `vector_result_t vector_pop(vector)`: pop last element from the vector following the LIFO policy;  
- `vector_result_t vector_clear(vector)`: logically reset the vector. That is, new pushes
will overwrite the memory;  
- `vector_result_t vector_destroy(vector)`: delete the vector;  
- `size_t vector_size(vector)`: return vector size (i.e., the number of elements);  
- `size_t vector_capacity(vector)`: return vector capacity (i.e., vector total size).

As you can see, most methods that operates on the `Vector` data type return a custom type called
`vector_result_t` which is defined as follows:

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

Each method that returns such type indicates whether the operation was successful or not by
setting the `status` field and by providing a descriptive message on the `message` field.
Just like for the `Map` data structure, if the operation was successful 
(that is, `status == VECTOR_OK`), you can either move on with the rest of the program
or read the returned value from the sum data type. 

## Sorting
The `Vector` data structure provides an efficient in-place sorting method called `vector_sort`
which uses a builtin [Quicksort](https://en.wikipedia.org/wiki/Quicksort) implementation. This
function requires an user-defined comparison procedure as its second parameter, which allows 
the caller to customize the sorting behavior. It must adhere to the following specification:

1. Must return `vector_order_t`, which is defined as follows:

```c
typedef enum {
    VECTOR_ORDER_LT = 0x0, // First element should come before the second
    VECTOR_ORDER_EQ, // The two elements are equivalent
    VECTOR_ORDER_GT // First element should come after the second
} vector_order_t;
```

and indicates the ordering relationship between any two elements.

2. Must accept two `const void*` parameters representing the two elements to compare;  
3. Must be self-contained and handle all its own resources.

Let's look at some examples; for instance, let's sort an integer array in ascending and
descending order:

```c
#include <stdio.h>
#include "src/vector.h"

vector_order_t cmp_int_asc(const void *x, const void *y) {
    int x_int = *(const int*)x;
    int y_int = *(const int*)y;

    if (x_int < y_int) return VECTOR_ORDER_LT;
    if (x_int > y_int) return VECTOR_ORDER_GT;

    return VECTOR_ORDER_EQ;
}

vector_order_t cmp_int_desc(const void *x, const void *y) {
    return cmp_int_asc(y, x);
}

/*
 * Compile with: gcc main.c src/vector.h
 * Output: Before sorting: -8 20 -10 125 34 9 
 *         After sorting (ascending order): -10 -8 9 20 34 125 
 *         After sorting (descending order): 125 34 20 9 -8 -10 
 */
int main(void) {
    vector_t *v = vector_new(5, sizeof(int)).value.vector;

    int values[] = { -8, 20, -10, 125, 34, 9 };
    for (size_t idx = 0; idx < 6; idx++) {
        vector_push(v, &values[idx]);
    }

    // Print unsorted array
    printf("Before sorting: ");
    for (size_t idx = 0; idx < vector_size(v); idx++) {
        printf("%d ", *(int*)vector_get(v, idx).value.element);
    }

    // Sort array in ascending order
    vector_sort(v, cmp_int_asc);

    // Print sorted array
    printf("\nAfter sorting (ascending order): ");
    for (size_t idx = 0; idx < vector_size(v); idx++) {
        printf("%d ", *(int*)vector_get(v, idx).value.element);
    }

    // Sort array in descending order
    vector_sort(v, cmp_int_desc);

    // Print sorted array
    printf("\nAfter sorting (descending order): ");
    for (size_t idx = 0; idx < vector_size(v); idx++) {
        printf("%d ", *(int*)vector_get(v, idx).value.element);
    }

    printf("\n");

    vector_destroy(v);

    return 0;
}
```

Obviously, you can use the `vector_sort` method on custom data types as well. For instance, let's suppose that you have a
struct representing employees and you want to sort them based on their age and based on their name (lexicographic sort):

```c
#include <stdio.h>
#include <string.h>
#include "src/vector.h"

typedef struct {
    char name[256];
    int age;
} Employee;

vector_order_t cmp_person_by_age(const void *x, const void *y) {
    const Employee *x_person = (const Employee*)x;
    const Employee *y_person = (const Employee*)y;

    if (x_person->age < y_person->age) return VECTOR_ORDER_LT;
    if (x_person->age > y_person->age) return VECTOR_ORDER_GT;

    return VECTOR_ORDER_EQ;
}

vector_order_t cmp_person_by_name(const void *x, const void *y) {
    const Employee *x_person = (const Employee*)x;
    const Employee *y_person = (const Employee*)y;

    const int result = strcmp(x_person->name, y_person->name);

    if(result < 0) return VECTOR_ORDER_LT;
    if(result > 0) return VECTOR_ORDER_GT;
    
    return VECTOR_ORDER_EQ;
}

/*
 * Compile with: gcc main.c src/vector.h
 * Output: Sort by age:
 *         Name: Marco, Age: 25
 *         Name: Alice, Age: 28
 *         Name: Bob, Age: 45
 * 
 *         Sort by name:
 *         Name: Alice, Age: 28
 *         Name: Bob, Age: 45
 *         Name: Marco, Age: 25
 */
int main(void) {
    vector_t *employees = vector_new(5, sizeof(Employee)).value.vector;

    Employee e1 = { .name = "Bob", .age = 45 };
    Employee e2 = { .name = "Alice", .age = 28 };
    Employee e3 = { .name = "Marco", .age = 25 };
    
    vector_push(employees, &e1);
    vector_push(employees, &e2);
    vector_push(employees, &e3);

    // Sort array by age
    vector_sort(employees, cmp_person_by_age);

    // Print sorted array
    printf("Sort by age:\n");
    for (size_t idx = 0; idx < vector_size(employees); idx++) {
        Employee *p = (Employee*)vector_get(employees, idx).value.element;
        printf("Name: %s, Age: %d\n", p->name, p->age);
    }

    // Sort array by name
    vector_sort(employees, cmp_person_by_name);
    
    // Print sorted array
    printf("\nSort by name:\n");
    for (size_t idx = 0; idx < vector_size(employees); idx++) {
        Employee *p = (Employee*)vector_get(employees, idx).value.element;
        printf("Name: %s, Age: %d\n", p->name, p->age);
    }

    vector_destroy(employees);

    return 0;
}
```

## Unit tests
Datum provides some unit tests for both the `Vector` and the `Map` data types. To run them, you can issue the following commands:

```sh
$ make clean all
$ ./test_vector
$ ./test_map
```

## License
This library is released under the GPLv3 license. You can find a copy of the license with this repository or by visiting
[the following link](https://choosealicense.com/licenses/gpl-3.0/).
