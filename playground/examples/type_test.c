#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// print(x) picks the right printf format from x's C type
#define print(x) _Generic((x), \
    int: print_int,            \
    double: print_float,       \
    bool: print_bool,          \
    char *: print_string,      \
    const char *: print_string)(x)

static inline void print_int(int x) { printf("%d", x); }
static inline void print_float(double x) { printf("%g", x); }
static inline void print_bool(bool x) { printf("%s", x ? "true" : "false"); }
static inline void print_string(const char *x) { printf("%s", x); }

// string + string; never freed, fine for short programs
static inline const char *concat(const char *a, const char *b)
{
    char *result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

int main(void)
{
    const char * expected_string_1;
    const char * expected_string_2;
    const char * expected_string_3;
    const char * expected_string_4;
    const char * expected_string_5;
    const char * expected_string_6;
    const char * expected_string_7;
    const char * expected_string_8;
    const char * expected_string_9;
    const char * expected_string_10;
    int expected_int_1;
    int expected_int_2;
    int expected_int_3;
    int expected_int_4;
    int expected_int_5;
    int expected_int_6;
    const char * expected_string_copy_1;
    int expected_int_copy_1;
    int expected_int_copy_2;
    int _t0;
    int expected_int_expr_1;
    int _t1;
    int expected_int_expr_2;
    int _t2;
    int expected_int_expr_3;
    int _t3;
    int _t4;
    int expected_int_expr_4;
    const char * _t5;
    const char * expected_string_expr_1;
    const char * _t6;
    const char * expected_string_expr_2;
    bool _t7;
    bool expected_bool_expr_1;
    bool _t8;
    bool expected_bool_expr_2;
    bool _t9;
    bool expected_bool_expr_3;
    bool _t10;
    bool _t11;
    bool _t12;
    bool expected_bool_expr_4;

    expected_string_1 = "test";
    expected_string_2 = "";
    expected_string_3 = " ";
    expected_string_4 = "123";
    expected_string_5 = "1.5";
    expected_string_6 = "true";
    expected_string_7 = "hello world";
    expected_string_8 = "line\nbreak";
    expected_string_9 = "quote \" inside";
    expected_string_10 = "back\\slash";
    expected_int_1 = 0;
    expected_int_2 = 1;
    expected_int_3 = 42;
    expected_int_4 = 1000000;
    expected_int_5 = 2147483647;
    expected_int_6 = 7;
    expected_string_copy_1 = expected_string_1;
    expected_int_copy_1 = expected_int_3;
    expected_int_copy_2 = expected_int_copy_1;
    _t0 = 1 + 2;
    expected_int_expr_1 = _t0;
    _t1 = 10 - 3;
    expected_int_expr_2 = _t1;
    _t2 = expected_int_3 * 2;
    expected_int_expr_3 = _t2;
    _t3 = 4 + 5;
    _t4 = _t3 * 2;
    expected_int_expr_4 = _t4;
    _t5 = concat("a", "b");
    expected_string_expr_1 = _t5;
    _t6 = concat(expected_string_1, "!");
    expected_string_expr_2 = _t6;
    _t7 = 1 < 2;
    expected_bool_expr_1 = _t7;
    _t8 = expected_int_3 == 42;
    expected_bool_expr_2 = _t8;
    _t9 = expected_int_1 != expected_int_2;
    expected_bool_expr_3 = _t9;
    _t10 = 1 < 2;
    _t11 = 3 > 2;
    _t12 = _t10 && _t11;
    expected_bool_expr_4 = _t12;

    return 0;
}
