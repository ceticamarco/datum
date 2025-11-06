# Datum [![](https://github.com/ceticamarco/datum/actions/workflows/datum.yml/badge.svg)](https://github.com/ceticamarco/datum/actions/workflows/datum.yml)
Datum is a collection of dynamic and generic data structures implemented from scratch in C with no external dependencies beyond
the standard library. It currently features:

- **Vector**: a growable, contiguous array supporting homogenous data types (both primitives and user-defined types);  
- **Map**: an associative array that handles generic heterogenous data types;  

To learn more about the memory model of this library as well as the technical details
on how to use it efficiently and safely, be sure to read [the design manual](docs/manual.pdf).

## Usage
At its simplest, you can use this library as follows:

### `Vector`

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
    vector_push(vec, &(int){6}); // Equivalent as above

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

### `Map`

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

    const Person bob = { .name = "Bob", .surname = "Smith", .age = 34 };

    // Add a key to the map
    map_add(map, "bob", (void*)&bob);

    // Retrieve 'Bob' and check if it exists
    map_result_t bob_res = map_get(map, "bob");
    if (bob_res.status == MAP_ERR_NOT_FOUND) {
        puts("This key does not exist.");
    } else {
        const Person *retr = (const Person*)bob_res.value.element;
        printf("Name: %s, Surname: %s, Age: %d\n", retr->name, retr->surname, retr->age);
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
As stated earlier, refer to [the design manual](docs/manual.pdf) for a comprehensive documentation of this library. Below, there's a quick
overview about the design choices behind Datum. While both structures use `void*` to represent values, the way they manage memory is orthogonally different
from one another. Let's start with the `Map` data type.

`Map` is an hash table implementation that uses open addressing with linear probing for collision resolution and the 
[FNV-1a algorithm](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function) as its hashing function. Resizing is performed automatically
by doubling the capacity when load factor exceeds 75%. The keys are **copied** by the hashmap. This means that the hashmap **owns** them and is responsible
to manage their memory. Values, on the other hand, **are stored as pointers**. This means that the hashmap **does NOT own** them and the caller is responsible
to manage their memory; this includes: allocate enough memory for them, ensure that the pointers remain valid for their whole lifecycle on the map, 
delete old values when updating a key and, if the values were heap-allocated, free them before removing them or before destroying the map.

`Vector`, instead, is a dynamic array with generic data type support. This means that you can store any kind of homogenous value on the data structure. As in the `Map`'s case,
resizing is performed automatically by increasing the capacity by 1.5 times when the array is full. The dynamic array copies the values upon insertion, thus it is responsible
for their allocation and their deletion.

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