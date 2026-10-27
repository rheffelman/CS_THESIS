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
    // @DECLARATIONS

    // @CODE

    return 0;
}
