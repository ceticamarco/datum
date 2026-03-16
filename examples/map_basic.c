/*
 * Basic map operations example.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../src/map.h"

int main(void) {
    // Create a new map
    map_result_t res = map_new();
    if (res.status != MAP_OK) {
        printf("Error while creating map: %s\n", res.message);
        return 1;
    }

    map_t *map = res.value.map;

    // Add some values
    const int x = 0xB00B5;
    const char *y = "Hello";

    map_result_t add_res = map_add(map, "x", (void*)&x);
    if (add_res.status != MAP_OK) {
        printf("Error while adding elements: %s\n", add_res.message);
        return 1;
    }

    add_res = map_add(map, "y", (void*)y);
    if (add_res.status != MAP_OK) {
        printf("Error while adding elements: %s\n", add_res.message);
        return 1;
    }

    // Print size and capacity
    printf("Map size (should be 2): %zu\n", map_size(map));
    printf("Map capacity (should be > 2): %zu\n\n", map_capacity(map));

    // Retrieve keys
    map_result_t get_res = map_get(map, "x");
    if (get_res.status != MAP_OK) {
        printf("Cannot retrieve map element 'x': %s\n", get_res.message);
        return 1;
    } else {
        const int *val = (const int*)get_res.value.element;
        printf("Key 'x' contains (should be 'B00B5'): %X\n", *val);
    }

    get_res = map_get(map, "y");
    if (get_res.status != MAP_OK) {
        printf("Cannot retrieve map element 'y': %s\n", get_res.message);
        return 1;
    } else {
        const char *val = (const char*)get_res.value.element;
        printf("Key 'y' contains (should be 'Hello') : %s\n\n", val);
    }

    // Update key
    const int new_x = 0xC0FFEE;
    map_result_t up_res = map_add(map, "x", (void*)&new_x);

    up_res = map_get(map, "x");
    if (get_res.status != MAP_OK) {
        printf("Cannot retrieve map element 'x': %s\n", get_res.message);
        return 1;
    } else {
        const int *val = (const int*)up_res.value.element;
        printf("Key 'x' (should be updated to 'C0FFEE'): %X\n\n", *val);
    }

    // Remove an element
    map_result_t rm_res = map_remove(map, "y");
    if (rm_res.status != MAP_OK) {
        printf("Cannot remove map element 'y': %s\n", rm_res.message);
        return 1;
    } else {
        printf("Map element 'y' removed (size should be 1): %zu\n\n", map_size(map));
    }

    // Clear the map
    map_result_t clear_res = map_clear(map);
    if (clear_res.status != MAP_OK) {
        printf("Cannot clear map: %s\n", clear_res.message);
        return 1;
    } else {
        printf("Map cleared (size should be 0): %zu\n", map_size(map));
    }

    printf("\n");

    // Delete the map
    map_result_t del_res = map_destroy(map);
    if (del_res.status != MAP_OK) {
        printf("Error while destroying the map: %s\n", del_res.message);
        return 1;
    }

    return 0;
}

