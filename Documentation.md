# Ry

Ry is a small programming language built for my CS thesis. A Ry program is a
list of statements that run top to bottom, each ending in `;` like C, C++ or
Rust. Control flow uses **labels** and **branches** instead of loops or
functions.

Ry compiles to C. An ANTLR grammar parses the source, a Python program lowers
the parse tree into a simple tuple IR and checks types, and the IR is printed
as C code that `gcc` turns into an executable.

## Project layout

```
compiler/
  Ry.g4             the grammar (source of truth for the syntax)
  generate.cmd      runs ANTLR on Ry.g4, writing the parser to generated/
  main.py           parse tree -> IR -> type check -> C
  template.c        C wrapper the generated code is inserted into (print, string concat)
  explore.py        explore(obj): prints what an ANTLR object is and what it has
  run.cmd           compile and run one .ry file
  examples/         sample programs
Documentation.md    this file
ProgressLog.md      weekly progress log
```

Related projects, kept separately:
- `ry-rust-interpreter`: the original Rust interpreter, now archived.
- `vscode-ry`: syntax highlighting for `.ry` files in VS Code.

## Building and running

You need Java (for ANTLR), Python 3 and `gcc`. The ANTLR jar is expected at
`%USERPROFILE%\.m2\repository\org\antlr\antlr4\4.13.2\antlr4-4.13.2-complete.jar`.

One-time setup, from `compiler/`:

```bat
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
generate.cmd
```

Run `generate.cmd` again whenever `Ry.g4` changes. Then:

```bat
run.cmd examples\1.ry
```

This prints the IR, writes `examples\1.c`, compiles it to `examples\1.exe`
and runs it.

## The language

### Statements

| Statement            | Example                      |
| -------------------- | ---------------------------- |
| declaration          | `let x = 5;`                 |
| expression           | `x = (x + 1);`, `print(x);`  |
| `if`                 | `if (x < 10) { ... }`        |
| `label`              | `label loop { ... }`         |
| `branch`             | `branch loop;`               |
| `fn` (not compiled yet) | `fn add { ... }`          |

### Types

Every variable has a fixed type, inferred from the value it is first given with
`let`. Assigning a value of a different type later is a compile error.

| Ry type  | Literals                         | C type         |
| -------- | -------------------------------- | -------------- |
| `int`    | `0`, `42`, `-7`, `007`           | `int`          |
| `float`  | `3.14`, `1.`, `.5`, `1e10`, `1.5e-3` | `double`   |
| `bool`   | `true`, `false`                  | `bool`         |
| `string` | `"hello\n"` (one line, `\` escapes) | `const char *` |

```
let x = 5;
let pi = 3.14;
let y = (x + pi);
```

Here `y` is a `float`, because adding an `int` and a `float` gives a `float`.

### Expressions

An expression is a single value or **one** binary operation: `a + b`. There is
no operator precedence. To combine operations, nest them in parentheses:

```
let Ts = ((H * T1) + ((1.0 - H) * (T1 + T2)));
```

`-` directly in front of a number literal makes it negative (`-0.5`).

| Operators                     | Works on                    | Result  |
| ----------------------------- | --------------------------- | ------- |
| `+ - * /`                     | `int`, `float`              | `int` if both are `int`, else `float` |
| `+`                           | two `string`s               | `string` (concatenation) |
| `< > == !=`                   | two numbers, or two of the same type | `bool` |
| `and or` (also `AND OR`)      | two `bool`s                 | `bool`  |
| `=`                           | variable, value of its type | assignment |

`not` / `NOT` are reserved words in the grammar but aren't supported by the
compiler yet.

### Function calls

`name(arg, arg, ...)` calls a function. Right now the only one is `print(x)`,
which prints any type without a newline.

### Labels and branches

`label name { ... }` marks a place in the program. `branch name;` jumps there.
A label's block runs when execution reaches it, and afterwards execution
continues with the statement that follows the block. Labels can be nested and
can be branched to from anywhere.

```
let i = 0;

branch one;

label one {
    print("Hello World!\n");
    i = (i + 1);
    if (i < 10) {
        branch one;
    }
}
```

### If statements

`if (condition) { ... }` runs the block only when the condition, a `bool`, is
true. The parentheses and braces are required. There is no `else` yet.

### Loops and functions

Ry has no loop statement. A loop is a label that branches back to itself:

```
let i = 0;
branch loop;
label loop {
    if (i < 100) {
        i = (i + 1);
        branch loop;
    }
}
```

This looks like recursion, but a branch is a jump (`goto` in the generated C),
so nothing is pushed onto the stack.

`fn name { ... }` parses but isn't compiled yet. Right now the body just runs
inline.

## How the compiler works

1. **Parse.** ANTLR builds a parse tree from `Ry.g4`.
2. **Lower to IR** (`statements` in `main.py`). Each statement becomes one or
   more tuples. Nested expressions are broken into temporaries `_t0, _t1, ...`,
   and each `if` gets a generated end label `_L0, _L1, ...`:

   | Tuple                                  | Meaning                     |
   | -------------------------------------- | --------------------------- |
   | `('LET', name, value)`                 | declare and set             |
   | `('ASSIGN', name, value)`              | set                         |
   | `('OP', temp, left, op, right)`        | `temp = left op right`      |
   | `('CALL', temp, name, [args])`         | `temp = name(args)`         |
   | `('LABEL', name)`                      | jump target                 |
   | `('BRANCH', name)`                     | jump                        |
   | `('BRANCH_IF_FALSE', cond, label)`     | jump unless `cond`          |

3. **Type check** (`type_deduction`). Gives every variable and temporary a type
   and rejects bad operations or assignments.
4. **Emit C** (`raise_stmts`). Declares every variable at the top of `main`, so
   a `goto` never skips a declaration, and writes one C line per tuple into
   `template.c`.

For example, this program:

```
let i = 0;
label one {
    i = (i + 1);
    if (i < 10) {
        branch one;
    }
}
```

becomes this IR:

```
('LET', 'i', '0', 'int')
('LABEL', 'one')
('OP', '_t0', 'i', '+', '1')
('ASSIGN', 'i', '_t0')
('OP', '_t1', 'i', '<', '10')
('BRANCH_IF_FALSE', '_t1', '_L0')
('BRANCH', 'one')
('LABEL', '_L0')
```

## Open questions

- **Functions.** What's the cleanest syntax? Possibly a mix of Rust and
  Python, with argument and return types inferred:
  ```
  fn add(a, b) {
      return a + b;
  }
  ```
- **Custom types.** How should structs or other user-defined objects work?
- **Comments.** The grammar doesn't support comments yet.
