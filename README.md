<div align="center">
    <h1>Datum</h1>
    <h6><i>Collection of dynamic and generic data structures.</i></h6>

[![](https://github.com/ceticamarco/datum/actions/workflows/datum.yml/badge.svg)](https://github.com/ceticamarco/datum/actions/workflows/datum.yml)
</div>

Datum is a collection of dynamic and generic data structures implemented from scratch in C with no external dependencies beyond
the standard library. It currently features:

- [**Vector**](/docs/vector.md): a growable, contiguous array of homogenous generic data types;  
- [**Map**](/docs/map.md): an associative array that handles generic heterogenous data types;  

## Usage
At its simplest, you can use this library as follows:

### `Vector`'s usage

```c
#include <stdio.h>
#include "src/vector.h"

/*
 * Compile with: gcc main.c src/vector.c
 * Output: First element: 5
 *         Head of vector: 6, size is now: 1 
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
    printf("Head of vector: %d, size is now: %zu\n", head, vector_size(vec));

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

## Documentation
For additional details about this library (internal design, memory
management, data ownership, etc.) go to the `docs/` folder.

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
