<div align="center">
    <h1>Datum</h1>
    <h6><i>Collection of dynamic and generic data structures.</i></h6>

[![](https://github.com/ceticamarco/datum/actions/workflows/gcc-build.yml/badge.svg)](https://github.com/ceticamarco/datum/actions/workflows/gcc-build.yml)
[![](https://github.com/ceticamarco/datum/actions/workflows/clang-build.yml/badge.svg)](https://github.com/ceticamarco/datum/actions/workflows/clang-build.yml)
</div>

Datum is a collection of dynamic and generic data structures implemented from scratch in C with no external dependencies beyond
the standard library. It currently features:

- [**Vector**](/docs/vector.md): a growable, contiguous array of homogenous generic data types;  
- [**Map**](/docs/map.md): an associative array that handles generic heterogenous data types;  
- [**BigInt**](/docs/bigint.md): a data type for arbitrary large integers.  

## Usage
At its simplest, you can use this library as follows:

### `Vector` usage

```c
#include <stdio.h>
#include "src/vector.h"

/*
 * Compile with: gcc main.c src/vector.c
 * Output: First element: 1
 *         Head of vector: 16, size is now: 1
 */ 

vector_order_t cmp_int_asc(const void *x, const void *y) {
    int x_int = *(const int*)x;
    int y_int = *(const int*)y;

    if (x_int < y_int) return VECTOR_ORDER_LT;
    if (x_int > y_int) return VECTOR_ORDER_GT;

    return VECTOR_ORDER_EQ;
}

void square(void *element, void *env) {
    (void)(env);
    int *value = (int*)element;
    *value = (*value) * (*value);
}

int is_even(const void *element, void *env) {
    (void)(env);
    int value = *(int*)element;

    return (value % 2) == 0;
}

void add(void *accumulator, const void *element, void *env) {
    (void)(env);
    *(int*)accumulator += *(int*)element;
}

int main(void) {
    // Create an integer vector of initial capacity equal to 5
    vector_t *vec = vector_new(5, sizeof(int)).value.vector;

    // Add some elements
    vector_push(vec, &(int){1}); // Equivalent as below
    int nums[] = {5, 2, 4, 3};
    for (int idx = 0; idx < 4; idx++) { vector_push(vec, &nums[idx]); }

    // Sort array in ascending order: [1, 2, 3, 4, 5]
    vector_sort(vec, cmp_int_asc);

    // Print 1st element
    const int first = *(int*)vector_get(vec, 0).value.element;
    printf("First element: %d\n", first);

    // Square elements: [1, 2, 3, 4, 5] -> [1, 4, 9, 16, 25]
    vector_map(vec, square, NULL);

    // Filter even elements: [1, 4, 9, 16, 25] -> [4, 16]
    vector_filter(vec, is_even, NULL);

    // Sume elements: [4, 16] -> 20
    int sum = 0;
    vector_reduce(vec, &sum, add, NULL);

    // Pop second element using LIFO policy
    const int head = *(int*)vector_pop(vec).value.element;
    printf("Head of vector: %d, size is now: %zu\n", head, vector_size(vec));

    // Remove vector from memory
    vector_destroy(vec);
    
    return 0;
}
```

### `Map` usage

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

### `BigInt` usage
```c
#include "src/bigint.h"

/*
 * Compile with: gcc -O3 main.c src/bigint.c src/vector.c
 * Output: 20000! = 1819206320230345134827641...
 * Time: 4.01s user 0.00s system 99% cpu 4.021 total
 */
int main(void) {
    const int n = 20000;
    bigint_t *fact = bigint_from_int(1).value.number;

    for (int idx = 2; idx <= n; idx++) {
        bigint_t *big_idx = bigint_from_int(idx).value.number;
        bigint_t *partial_fact = bigint_prod(fact, big_idx).value.number;

        bigint_destroy(fact);
        bigint_destroy(big_idx);
        fact = partial_fact;
    }

    bigint_printf("%d! = %B\n", n, fact);

    bigint_destroy(fact);
    return 0;
}
```

For a more exhaustive example, refer to the `usage.c` file. There, you will find a program with proper error management
and a sample usage for every available method. To run it, first issue the following command:

```sh
$ make clean all
```

This will compile the library as well as the `usage.c` file, the unit tests and the benchmark. After that, you can run it by typing `./usage`.

> [!NOTE]
> This project is primarily developed for learning purposes and was not created with industrial
> or production use in mind. As such, it is not intended to compete with any existing C library.
> In particular, the big number implementation does not aim to match the design, the maturity and
> the performance of established solutions such as the
> GNU Multiple Precision Arithmetic Library (GMP).

## Documentation
For additional details about this library (internal design, memory
management, data ownership, etc.) go to the [docs folder](/docs).

## Unit tests
Datum provides some unit tests for `Vector`, `Map` and `BigInt`. To run them, you can issue the following commands:

```sh
$ make clean all
$ ./test_vector
$ ./test_map
$ ./test_bigint
```

## Benchmark
Under the [`benchmark/`](/benchmark/) folder, you can find a simple benchmark program that stress the `Vector` and the `Map` data structures. You can run it by issuing the following command:

```sh
$ ./benchmark_datum
Computing Vector average time...average time: 18 ms
Computing Map average time...average time: 31 ms
```


## License
This library is released under the GPLv3 license. You can find a copy of the license with this repository or by visiting
[the following link](https://choosealicense.com/licenses/gpl-3.0/).
