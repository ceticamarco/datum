/*
 * Unit tests for Vector data type
*/

#define TEST(NAME) do { \
    printf("Running test_%s...", #NAME); \
    test_##NAME(); \
    printf(" PASSED\n"); \
} while(0)

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../src/vector.h"

// Create a new vector
void test_vector_new(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    assert(res.value.vector != NULL);
    assert(vector_size(res.value.vector) == 0);
    assert(vector_capacity(res.value.vector) == 5);

    vector_destroy(res.value.vector);
}

// Create a vector with zero capacity
void test_vector_new_zcap(void) {
    vector_result_t res = vector_new(0, sizeof(int));

    assert(res.status == VECTOR_ERR_ALLOCATE);
}

// Push elements to vector
void test_vector_push(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int x = 42, y = 84;

    vector_result_t x_res = vector_push(v, &x);
    assert(x_res.status == VECTOR_OK);
    assert(vector_size(v) == 1);

    vector_result_t y_res = vector_push(v, &y);
    assert(y_res.status == VECTOR_OK);
    assert(vector_size(v) == 2);

    vector_destroy(v);
}

// Trigger vector reallocation
void test_vector_push_realloc(void) {
    vector_result_t res = vector_new(1, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    for (int i = 0; i < 5; i++) {
        vector_result_t push_res = vector_push(v, &i);
        assert(push_res.status == VECTOR_OK);
    }

    assert(vector_size(v) == 5);
    assert(vector_capacity(v) > 5);

    vector_destroy(v);
}

// Get vector elements
void test_vector_get(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int val = 123;
    vector_push(v, &val);

    vector_result_t get_res = vector_get(v, 0);
    assert(get_res.status == VECTOR_OK);
    assert(*(int*)get_res.value.element == 123);

    vector_destroy(v);
}

// Test out of bounds
void test_vector_get_ofb(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int val = 123;
    vector_push(v, &val);

    vector_result_t get_res = vector_get(v, 10);
    assert(get_res.status != VECTOR_OK);

    vector_destroy(v);
}

// Sort integers in ascending order
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

void test_vector_sort_int_asc(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int values[] = { 25, 4, 12, -7, 25, 71, 1, 6 };
    for (size_t idx = 0; idx < 8; idx++) {
        vector_push(v, &values[idx]);
    }

    vector_result_t sort_res = vector_sort(v, cmp_int_asc);
    assert(sort_res.status == VECTOR_OK);

    const int expected[] = { -7, 1, 4, 6, 12, 25, 25, 71 };

    const size_t sz = vector_size(v);
    for (size_t idx = 0; idx < sz; idx++) {
        int *val = (int*)vector_get(v, idx).value.element;
        assert(*val == expected[idx]);
    }

    vector_destroy(v);
}

void test_vector_sort_int_desc(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int values[] = { 25, 4, 12, -7, 25, 71, 1, 6 };
    for (size_t idx = 0; idx < 8; idx++) {
        vector_push(v, &values[idx]);
    }

    vector_result_t sort_res = vector_sort(v, cmp_int_desc);
    assert(sort_res.status == VECTOR_OK);

    const int expected[] = { 71, 25, 25, 12, 6, 4, 1, -7 };

    const size_t sz = vector_size(v);
    for (size_t idx = 0; idx < sz; idx++) {
        int *val = (int*)vector_get(v, idx).value.element;
        assert(*val == expected[idx]);
    }

    vector_destroy(v);
}

// Sort strings in descending order
vector_order_t cmp_string_desc(const void *x, const void *y) {
    const char *x_str = *(const char* const*)x;
    const char *y_str = *(const char* const*)y;

    // strcmp() returns an integer indicating the result of the comparison, as follows:
    //  - 0, if the s1 and s2 are equal;
    //  - a negative value if s1 is less than s2;
    //  - a positive value if s1 is greater than s2.
    // for descending order, just invert the result
    const int result = strcmp(x_str, y_str);

    if (result < 0) return VECTOR_ORDER_GT;
    if (result > 0) return VECTOR_ORDER_LT;

    return VECTOR_ORDER_EQ;
}

void test_vector_sort_string(void) {
    vector_result_t res = vector_new(5, sizeof(char*));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    const char *values[] = { "embedded", "system-programming", "foo", "bar", "hello", "world!" };
    for (size_t idx = 0; idx < 6; idx++) {
        vector_push(v, &values[idx]);
    }

    vector_result_t sort_res = vector_sort(v, cmp_string_desc);
    assert(sort_res.status == VECTOR_OK);

    const char *expected[] = { "world!", "system-programming", "hello", "foo", "embedded", "bar"};

    const size_t sz = vector_size(v);
    for (size_t idx = 0; idx < sz; idx++) {
        const char *val = *(const char**)vector_get(v, idx).value.element;
        assert(!strcmp(val, expected[idx]));
    }

    vector_destroy(v);
}

// Sort vector with custom data type
typedef struct {
    const char *name;
    int age;
} Person;

vector_order_t cmp_person_by_age(const void *x, const void *y) {
    const Person *x_person = (const Person*)x;
    const Person *y_person = (const Person*)y;

    if (x_person->age < y_person->age) return VECTOR_ORDER_LT;
    if (x_person->age > y_person->age) return VECTOR_ORDER_GT;

    return VECTOR_ORDER_EQ;
}

vector_order_t cmp_person_by_name(const void *x, const void *y) {
    const Person *x_person = (const Person*)x;
    const Person *y_person = (const Person*)y;

    const int result = strcmp(x_person->name, y_person->name);

    if(result < 0) return VECTOR_ORDER_LT;
    if(result > 0) return VECTOR_ORDER_GT;
    
    return VECTOR_ORDER_EQ;
}

void test_vector_sort_struct_by_age(void) {
    vector_result_t res = vector_new(5, sizeof(Person));

    assert(res.status == VECTOR_OK);
    vector_t *people = res.value.vector;

    Person p1 = { .name = "Bob", .age = 45 };
    Person p2 = { .name = "Alice", .age = 28 };
    Person p3 = { .name = "Marco", .age = 25 };

    vector_push(people, &p1);
    vector_push(people, &p2);
    vector_push(people, &p3);

    vector_result_t sort_res = vector_sort(people, cmp_person_by_age);
    assert(sort_res.status == VECTOR_OK);

    Person expected[] = {
        { .name = "Marco", .age = 25 },
        { .name = "Alice", .age = 28 },
        { .name = "Bob", .age = 45 }
    };

    const size_t sz = sizeof(expected) / sizeof(expected[0]);
    for (size_t idx = 0; idx < sz; idx++) {
        Person *p = (Person*)vector_get(people, idx).value.element;
        assert(!strcmp(p->name, expected[idx].name));
        assert(p->age == expected[idx].age);
    }

    vector_destroy(people);
}

void test_vector_sort_struct_by_name(void) {
    vector_result_t res = vector_new(5, sizeof(Person));

    assert(res.status == VECTOR_OK);
    vector_t *people = res.value.vector;

    Person p1 = { .name = "Sophia", .age = 45 };
    Person p2 = { .name = "Robert", .age = 28 };
    Person p3 = { .name = "Barbara", .age = 25 };
    Person p4 = { .name = "Christopher", .age = 65 };
    Person p5 = { .name = "Paul", .age = 53 };

    vector_push(people, &p1);
    vector_push(people, &p2);
    vector_push(people, &p3);
    vector_push(people, &p4);
    vector_push(people, &p5);

    vector_result_t sort_res = vector_sort(people, cmp_person_by_name);
    assert(sort_res.status == VECTOR_OK);

    Person expected[] = {
        { .name = "Barbara", .age = 25 },
        { .name = "Christopher", .age = 65 },
        { .name = "Paul", .age = 53 },
        { .name = "Robert", .age = 28 },
        { .name = "Sophia", .age = 45 }
    };

    const size_t sz = vector_size(people);
    for (size_t idx = 0; idx < sz; idx++) {
        Person *p = (Person*)vector_get(people, idx).value.element;
        assert(!strcmp(p->name, expected[idx].name));
        assert(p->age == expected[idx].age);
    }

    vector_destroy(people);
}

// Set vector element
void test_vector_set(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int x = 123, y = 999;
    vector_push(v, &x);

    vector_result_t set_res = vector_set(v, 0, &y);
    assert(set_res.status == VECTOR_OK);

    vector_result_t get_res = vector_get(v, 0);
    assert(*(int*)get_res.value.element == 999);

    vector_destroy(v);
}

// Set vector element out of bounds
void test_vector_set_ofb(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int x = 10, y = 999;
    vector_push(v, &x);

    vector_result_t set_res = vector_set(v, 10, &y);
    assert(set_res.status != VECTOR_OK);

    vector_destroy(v);
}

// Pop element from vector
void test_vector_pop(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    int x = 10, y = 20;
    vector_push(v, &x);
    vector_push(v, &y);

    assert(vector_size(v) == 2);

    vector_result_t pop_res = vector_pop(v);
    assert(pop_res.status == VECTOR_OK);
    assert(*(int*)pop_res.value.element == 20);
    assert(vector_size(v) == 1);

    vector_destroy(v);
}

// Test pop element from empty vector
void test_vector_pop_empty(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    vector_result_t pop_res = vector_pop(v);
    assert(pop_res.status != VECTOR_OK);

    vector_destroy(v);
}

// Clear vector
void test_vector_clear(void) {
    vector_result_t res = vector_new(5, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    for (int i = 0; i < 5; i++) {
        vector_push(v, &i);
    }

    assert(vector_size(v) == 5);

    vector_result_t clear_res = vector_clear(v);
    assert(clear_res.status == VECTOR_OK);
    assert(vector_size(v) == 0);
    // Capacity should be unchanged, this is by design!
    assert(vector_capacity(v) == 5);

    vector_destroy(v);
}

// Multiple operations in sequence (push, set, pop and clear)
void test_vector_sequence(void) {
    vector_result_t res = vector_new(2, sizeof(int));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    // push
    for (int i = 0; i < 5; i++) {
        vector_push(v, &i);
    }

    // Set
    int new_val = 0xBABE;
    vector_set(v, 2, &new_val);

    vector_result_t get_res = vector_get(v, 2);
    assert(*(int*)get_res.value.element == 0xBABE);

    // Pop
    vector_pop(v);
    assert(vector_size(v) == 4);

    // Clear
    vector_clear(v);
    assert(vector_size(v) == 0);

    vector_destroy(v);
}

// Vector with chars
void test_vector_char(void) {
    vector_result_t res = vector_new(5, sizeof(char));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;

    char x = 'A', y = 'B', z = 'C';
    vector_push(v, &x);
    vector_push(v, &y);
    vector_push(v, &z);

    vector_result_t get_res = vector_get(v, 1);
    assert(*(char *)get_res.value.element == 'B');

    vector_destroy(v);
}

// Test vector with product data type
typedef struct {
    int x;
    int y;
} Point;

void test_vector_struct(void) {
    vector_result_t res = vector_new(5, sizeof(Point));

    assert(res.status == VECTOR_OK);
    vector_t *v = res.value.vector;
   
    Point p1 = {10, 20};
    Point p2 = {30, 40};

    vector_push(v, &p1);
    vector_push(v, &p2);

    vector_result_t get_res = vector_get(v, 0);
    Point *retr = (Point*)get_res.value.element;
    assert(retr->x == 10);
    assert(retr->y == 20);

    vector_destroy(v);
}

int main(void) {
    printf("=== Running Vector unit tests ===\n\n");

    TEST(vector_new);
    TEST(vector_new_zcap);
    TEST(vector_push);
    TEST(vector_push_realloc);
    TEST(vector_get);
    TEST(vector_get_ofb);
    TEST(vector_sort_int_asc);
    TEST(vector_sort_int_desc);
    TEST(vector_sort_string);
    TEST(vector_sort_struct_by_age);
    TEST(vector_sort_struct_by_name);
    TEST(vector_set);
    TEST(vector_set_ofb);
    TEST(vector_pop);
    TEST(vector_pop_empty);
    TEST(vector_clear);
    TEST(vector_sequence);
    TEST(vector_char);
    TEST(vector_struct);

    printf("\n=== All tests passed! ===\n");
    
    return 0;
}
