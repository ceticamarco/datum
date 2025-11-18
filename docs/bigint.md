# BigInt Technical Details
In this document you can find a quick overview of the technical
aspects (internal design, memory layout, etc.) of the `BigInt` data structure. 

`BigInt` is a data type for arbitrary precision arithmetic that supports addition,
subtraction, multiplication, division and modulo operations on signed integers of unlimited size. Internally, it uses
the `Vector` data structure to represent big numbers using the following layout:

```
Number:     2485795518678991171206065
Internally: [ 171206065, 518678991, 2485795 ]
                /            |          \
               /             |           \
           digit[0]      digit[1]      digit[2]
            (LSB)                        (MSB)
```

That is, each element of the vector stores 9 digits in base $10^9$ using
**little-endian order**. Each such digits can therefore store values from `0` up to
`999,999,999`.

This scheme maps to the following structure:

```c
typedef struct {
    vector_t *digits;
    bool is_negative;
} bigint_t;
```

where the `digits` array stores the representation in base $10^9$ of the big integer
and the boolean `is_negative` variable denotes its sign.

The `BigInt` data structure supports the following methods:

- `bigint_result_t bigint_from_int(value)`: create a big integer from a primitive `int` type;  
- `bigint_result_t bigint_from_string(string_num)`: create a big integer from a C string;  
- `bigint_result_t bigint_to_string(number)`: convert a big integer to a C string;  
- `bigint_result_t bigint_clone(number)`:  clone a big integer;  
- `bigint_result_t bigint_compare(x, y)`: compare two big integers, returning either `-1`, `0` or `1` if the first is less than, equal than or greater than the second, respectively;  
- `bigint_result_t bigint_add(x, y)`: add two big integers together in $\mathcal{O}(n)$;  
- `bigint_result_t bigint_sub(x, y)`: subtract two big integers in $\mathcal{O}(n)$;  
- `bigint_result_t bigint_prod(x, y)`: multiply two big integers using Karatsuba's algorithm in $\mathcal{O}(n^{1.585})$;  
- `bigint_result_t bigint_divmod(x, y)`: divide two big integers using *long division* algorithm in $\mathcal{O}(n^2)$, returning both the quotient and the remainder;  
- `bigint_result_t bigint_mod(x, y)`: computes modulo of two big integers using *long division* algorithm in $\mathcal{O}(n^2)$;  
- `bigint_result_t bigint_destroy(number)`: delete the big number;  
- `bigint_result_t bigint_printf(format, ...)`: `printf` wrapper that introduces the `%B` placeholder to print big numbers. It supports variadic parameters.

As you can see by the previous function signatures, methods that operate on the
`BigInt` data type return a custom type called `bigint_result_t` which is defined as
follows:

```c
typedef enum {
    BIGINT_OK = 0x0,
    BIGINT_ERR_ALLOCATE,
    BIGINT_ERR_DIV_BY_ZERO,
    BIGINT_ERR_INVALID
} bigint_status_t;

typedef struct {
    bigint_t *quotient;
    bigint_t *remainder;
} div_result_t;

typedef struct {
    bigint_status_t status;
    uint8_t message[RESULT_MSG_SIZE];
    union {
        bigint_t *number;
        div_result_t division;
        int8_t compare_status;
        char *string_num;
    } value;
} bigint_result_t;
```

Each method that returns such type indicates whether the operation was successful or not
by setting the `status` field and by providing a descriptive message on the `message`
field. If the operation was successful (that is, `status == BIGINT_OK`), you can either
move on with the rest of the program or read the returned value from the sum data type.
Of course, you can choose to ignore the return value (if you're brave enough :D) as 
illustrated in the first part of the README.

The sum data type (i.e., the `value` union) defines four different variables. Each
of them has an unique scope as described below:

- `number`: result of arithmetical, cloning and creating functions;  
- `division`: result of `bigint_divmod`;  
- `compare_status`: result of `bigint_compare`;  
- `string_num`: result of `bigint_to_string`.

