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
    int expected_int_1;
    int expected_int_2;
    double expected_float_1;
    double expected_float_2;
    double expected_float_3;
    double expected_float_4;
    double expected_float_5;
    double expected_float_6;
    double expected_float_7;
    double expected_float_8;
    double expected_float_9;
    double expected_float_10;
    double expected_float_11;
    double expected_float_12;
    bool expected_bool_1;
    bool expected_bool_2;
    double expected_float_copy_1;
    double _t0;
    double expected_float_expr_1;
    double _t1;
    double expected_float_expr_2;
    double _t2;
    double expected_float_expr_3;
    bool _t3;
    bool expected_bool_expr_1;

    expected_int_1 = -7;
    expected_int_2 = -2147483648;
    expected_float_1 = .1;
    expected_float_2 = 0.1;
    expected_float_3 = 1.0;
    expected_float_4 = 1.;
    expected_float_5 = -0.5;
    expected_float_6 = -.5;
    expected_float_7 = 3.14159;
    expected_float_8 = 0.0;
    expected_float_9 = 1e10;
    expected_float_10 = 1.5e-3;
    expected_float_11 = 2E3;
    expected_float_12 = 123456789.123456789;
    expected_bool_1 = true;
    expected_bool_2 = false;
    expected_float_copy_1 = expected_float_1;
    _t0 = 1.5 + 2.5;
    expected_float_expr_1 = _t0;
    _t1 = 1 + .5;
    expected_float_expr_2 = _t1;
    _t2 = expected_int_1 * expected_float_7;
    expected_float_expr_3 = _t2;
    _t3 = expected_bool_1 || expected_bool_2;
    expected_bool_expr_1 = _t3;

    return 0;
}
