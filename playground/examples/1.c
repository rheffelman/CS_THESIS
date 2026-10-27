#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

static inline const char *concat(const char *a, const char *b)
{
    char *result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

int main(void)
{
    int i;
    int y;
    const char * hw;
    int _t1;
    bool _t2;

    i = 0;
    y = 10;
    goto one;
    one:;
    hw = "Hello World!\n";
    print(hw);
    _t1 = i + 1;
    i = _t1;
    _t2 = i < 10;
    if (!_t2) goto _L0;
    goto one;
    _L0:;

    return 0;
}
