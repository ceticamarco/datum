# Vector Technical Details
In this document you can find a quick overview of the technical
aspects (internal design, memory layout, etc.) of the `Vector` data structure. 

`Vector` is a dynamic array with generic data type support; this means that you can store
any kind of homogenous value on this data structure. Resizing is performed automatically
by increasing the capacity by 1.5 times when the array becomes full. Internally, this 
data structure is represented by the following layout:

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
- `vector_result_t vector_sort(vector, cmp)`: sort vector using `cmp` function;  
- `vector_result_t vector_pop(vector)`: pop last element from the vector following the LIFO policy;  
- `vector_result_t vector_map(vector, callback, env)`: apply `callback` function to vector (in-place);  
- `vector_result_t vector_filter(vector, callback, env)`: filter vector using `callback` (in-place);  
- `vector_result_t vector_reduce(vector, accumulator, callback, env)`: fold/reduce vector using `callback`;  
- `vector_result_t vector_clear(vector)`: logically reset the vector. That is, new pushes will overwrite the memory;  
- `vector_result_t vector_destroy(vector)`: delete the vector;  
- `size_t vector_size(vector)`: return vector size (i.e., the number of elements);  
- `size_t vector_capacity(vector)`: return vector capacity (i.e., vector total size).

As you can see from the previous function signatures, most methods that operate
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
ignore the return value (if you're brave enough :D) as illustrated on the first part of the README.

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
- The `env` argument is an optional parameter to pass the external environment to the callback function. It is used to mock the behavior of closures, where
the lexical environment is captured when the closure is created;  
- Callback functions must be self-contained and handle all their resources. Additionally, they are responsible for ensuring their operations
don't cause any undefined behavior.

Let's look at an example:

```c
#include <stdio.h>
#include "src/vector.h"

// Callback functions
void square(void *element, void *env);
int is_even(const void *element, void *env);
void add(void *accumulator, const void *element, void *env);

int main(void) {
    // Create an integer vector of initial capacity equal to 5
    vector_t *vec = vector_new(5, sizeof(int)).value.vector;

    int nums[] = {1, 2, 3, 4, 5};
    for (int idx = 0; idx < 5; idx++) {
        vector_push(vec, &nums[idx]);
    }

    // Square elements: [1, 2, 3, 4, 5] -> [1, 4, 9, 16, 25]
    vector_map(vec, square, NULL);
    for (int idx = 0; idx < 5; idx++) {
        printf("%d ", *(int *)vector_get(vec, idx).value.element);
    }
    putchar('\n');

    // Filter even elements: [1, 4, 9, 16, 25] -> [4, 16]
    vector_filter(vec, is_even, NULL);
    for (int idx = 0; idx < 2; idx++) {
        printf("%d ", *(int *)vector_get(vec, idx).value.element);
    }
    putchar('\n');

    // Sum elements: [4, 16] -> 20
    int sum = 0;
    vector_reduce(vec, &sum, add, NULL);
    printf("%d\n", sum);

    vector_destroy(vec);

    return 0;
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
```

## Sorting
As indicated in the [its documentation](/docs/vector.md), the `Vector` data type
provides an efficient in-place sorting function called `vector_sort` that uses
a builtin implementation of the [Quicksort algorithm](https://en.wikipedia.org/wiki/Quicksort). This method requires an user-defined comparison procedure which allows the
caller to customize the sorting behavior. 

The comparison procedure must adhere to the following specification:

1. Must return `vector_order_t`, which is defined as follows:

```c
typedef enum {
    VECTOR_ORDER_LT = 0x0, // First element should come before the second
    VECTOR_ORDER_EQ, // The two elements are equivalent
    VECTOR_ORDER_GT // First element should come after the second
} vector_order_t;
```

and indicates the ordering relationship between any two elements.

2. Must accept two `const void*` parameters representing two elements to compare;  
3. Must be self-contained and handle all its resources. Additionally, it's responsible for ensuring its operations don't cause any undefined behavior.

Let's look at some examples. For instance, let's say that we want to sort an array
of integers in ascending and descending order:

```c
#include <stdio.h>
#include "src/vector.h"

vector_order_t cmp_int_asc(const void *x, const void *y) {
    const int x_int = *(const int*)x;
    const int y_int = *(const int*)y;

    if (x_int < y_int) return VECTOR_ORDER_LT;
    if (x_int > y_int) return VECTOR_ORDER_GT;

    return VECTOR_ORDER_EQ;
}

vector_order_t cmp_int_desc(const void *x, const void *y) {
    return cmp_int_asc(y, x);
}

/*
 * Compile with: gcc main.c src/vector.c
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

    const size_t sz = vector_size(v);

    // Print unsorted array
    printf("Before sorting: ");
    for (size_t idx = 0; idx < sz; idx++) {
        printf("%d ", *(int*)vector_get(v, idx).value.element);
    }

    // Sort array in ascending order
    vector_sort(v, cmp_int_asc);

    // Print sorted array
    printf("\nAfter sorting (ascending order): ");
    for (size_t idx = 0; idx < sz; idx++) {
        printf("%d ", *(int*)vector_get(v, idx).value.element);
    }

    // Sort array in descending order
    vector_sort(v, cmp_int_desc);

    // Print sorted array
    printf("\nAfter sorting (descending order): ");
    for (size_t idx = 0; idx < sz; idx++) {
        printf("%d ", *(int*)vector_get(v, idx).value.element);
    }

    printf("\n");

    vector_destroy(v);

    return 0;
}
```

Obviously, you can use the `vector_sort` method on custom data type as well. 
For instance, let's suppose that you have a structure representing the employees of
a company and you wish to sort them based on their age and on their name (lexicographic sort):

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
 * Compile with: gcc main.c src/vector.c
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

    const size_t sz = vector_size(employees);

    // Print sorted array
    printf("Sort by age:\n");
    for (size_t idx = 0; idx < sz; idx++) {
        Employee *p = (Employee*)vector_get(employees, idx).value.element;
        printf("Name: %s, Age: %d\n", p->name, p->age);
    }

    // Sort array by name
    vector_sort(employees, cmp_person_by_name);
    
    // Print sorted array
    printf("\nSort by name:\n");
    for (size_t idx = 0; idx < sz; idx++) {
        Employee *p = (Employee*)vector_get(employees, idx).value.element;
        printf("Name: %s, Age: %d\n", p->name, p->age);
    }

    vector_destroy(employees);

    return 0;
}
```
