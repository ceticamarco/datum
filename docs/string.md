# String Technical Details
In this document you can find a quick overview of the technical 
aspects (internal design, memory layout, etc.) of the `String` data structure.

`String` is an immutable string data type with partial UTF-8 support.
This means that methods return a new string instance rather than modifying the string in-place. 
Internally, this data structure is represented by the following layout:

```c
typedef struct {
    char *data;
    size_t byte_size;
    size_t byte_capacity;
    size_t char_count;
} string_t;
```

where the `data` variable represents the actual string (represented as a pointer to `char`),
the `byte_size` variable indicates the actual size (in bytes) of the string, the
`byte_capacity` variable represents the total number of allocated memory (in bytes) and the
`char_count` variable represent the number of logical characters, that is the number of
symbols.

As mentioned earlier, this library provides partial UTF-8 support. It is able to recognize
UTF-8 byte sequences as individual Unicode code points, which allows it to correctly distinguish
between byte length and character count. It fully supports Unicode symbols and emojis, while
remaining backward compatible with ASCII strings.

However, this data structure does not support localization. In particular, it does not perform
locale-aware conversion; for instance, uppercase/lowercase transformations are limited to ASCII
characters only. As a result, the German scharfes S (`ß`) is not convert to `SS`, the Spanish
`Ñ` is not converted to `ñ` and the Italian `é` (and its variants) is not treated as a single
symbol, but rather as a base letter combined with an accent.

At the time being, `String` supports the following methods:

- `string_result_t string_new(c_str)`: create a new string;  
- `string_result_t string_clone(str)`: clone an existing string;  
- `string_result_t string_concat(x, y)`: concatenate two strings together;  
- `string_result_t string_contains(haystack, needle)`: search whether the `haystack` string contains `needle`;  
- `string_result_t string_slice(str, start, end)`: return a slice (a new string) from `str` between `start` and `end` indices (inclusive);  
- `string_result_t string_eq(x, y, case_sensitive)`: check whether `x` and `y` are equal;  
- `string_result_t string_get_at(str, position)`: get the UTF-8 symbol indexed by `position` from `str`;  
- `string_result_t string_set_at(str, position, utf8_char)`: write a UTF-8 symbol into `str` at index `position`;  
- `string_result_t string_to_lower(str)`: convert a string to lowercase;  
- `string_result_t string_to_upper(str)`: convert a string to uppercase;  
- `string_result_t string_reverse(str)`: reverse a string;  
- `string_result_t string_trim(str)`: remove leading and trailing white space from a string;   
- `string_result_t string_split(str, delim)`: split a string into an array of `string_t` by specifying a separator;  
- `string_result_t string_destroy(str)`: remove a string from memory;  
- `string_result_t string_split_destroy(split, count)`: remove an array of strings from memory;  
- `size_t string_size(str)`: return string character count.  

As you can see from the previous function signatures, most methods that operate on the `String`
data type return a custom type called `string_result_t` which is defined as follows:

```c
typedef enum {
    STRING_OK = 0x0,
    STRING_ERR_ALLOCATE,
    STRING_ERR_INVALID,
    STRING_ERR_INVALID_UTF8,
    STRING_ERR_OVERFLOW
} string_status_t;

typedef struct {
    string_status_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        string_t *string; // For new, clone, slice, reverse, trim
        char *symbol; // For get_at
        int64_t idx; // For contains
        bool is_equ; // For comparison
        struct { // For split
            string_t **strings;
            size_t count;
        } split;
    } value;
} string_result_t;
```

Each method that returns such type indicates whether the operation was successful or not
by setting the `status` field and by providing a descriptive message on the `message`
field. If the operation was successful (that is, `status == STRING_OK`) you can either
move on with the rest of your program or read the returned value from the sum data type.
Of course, you can choose to ignore the return value (if you're brave enough :D) as illustrated
on the first part of the README.

The sum data type (i.e., the `value` union) defines five different variables.
Each of them has an unique scope as described below:

- `string`: result of `new`, `clone`, `slice`, `reverse` and `trim` functions;  
- `symbol`: result of `get_at` function;  
- `idx`: result of `contains` function;  
- `is_eq`: result of `equ` function. It's true when two strings are equal, false otherwise;  
- `split`: result of `split` function. It contains an array of `string_t` and its number of elements.  
