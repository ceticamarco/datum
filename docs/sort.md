# Sorting
As indicated in the [its documentation](/docs/vector.md), the `Vector` data type
provides an efficient in-place sorting function called `vector_sort` that uses
a builtin implementation of the [Quicksort algorithm](https://en.wikipedia.org/wiki/Quicksort). This method requires an user-defined comparison procedure which allows the
caller to customize the sorting behavior. The comparison procedure must adhere to the
following specification:

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
3. Must be self-contained and handle all its own resources.

Let's look at some examples. For instance, let's say that we want to sort an array
of integers in ascending and descending order:

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