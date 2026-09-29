# ANTLR playground

A place to write grammars, see exactly how ANTLR tokenizes and parses input, and
write C++ code (visitors/listeners) that does something with the parse tree.

Run everything from this `playground` folder, in PowerShell or cmd.

## Quick start

```
.\play Calc examples\basic.calc              # parse a file: prints the tree, then runs main.cpp's code
.\play Calc examples\basic.calc --tokens     # also show what the lexer produced
.\play Calc -e "1 + 2 * 3" --rule expr       # parse text directly, starting from the `expr` rule
.\play Calc --repl                           # interactive: type a line, see its tree
.\play Ry examples\basic.ry                  # your thesis language
.\play Calc --help                           # all options
```

`play` regenerates the C++ parser whenever the `.g4` changes and rebuilds
whatever changed, so the loop is: **edit the grammar or main.cpp → rerun `play`**.

## What's here

```
playground/
├── play.cmd                  build + run a grammar
├── new.cmd                   create a new grammar from templates/
├── grammars/
│   ├── Calc/                 calculator; evaluates the tree with a VISITOR
│   │   ├── Calc.g4           the grammar
│   │   ├── main.cpp          your C++ code: start rules + what to do with the tree
│   │   └── examples/         sample inputs
│   └── Ry/                   your thesis language; checks the tree with a LISTENER
├── common/                   the shared driver (tree printer, error display, CLI)
├── templates/                starting point used by new.cmd
└── build/                    generated + compiled output (git-ignored, safe to delete)
    └── generated/<Name>/     the C++ that ANTLR generated from <Name>.g4
```

## Options

| Option | What it shows |
|---|---|
| *(none)* | Parse tree, then the output of the code in `main.cpp` |
| `-t`, `--tokens` | The token table: stage 1, what the **lexer** turned the characters into |
| `-l`, `--lisp` | The tree in LISP form, the same as `antlr4-parse -tree` |
| `--trace` | Logs **live, while parsing**, every rule entered/exited and every token matched |
| `--diag` | Warns about ambiguities in your grammar (input that could parse two ways) |
| `-r NAME`, `--rule NAME` | Start rule. Any rule works; registered ones also run your code |
| `--rules` | Lists the grammar's parser rules and token types |
| `--no-tree`, `--no-run`, `--no-color` | Turn things off |
| `-i`, `--repl` | Interactive mode. Inside it, `:tokens`, `:trace`, `:lisp`… toggle options and `:rule expr` switches the start rule |

Input files are looked up relative to where you are, and also inside
`grammars/<Name>/`, so `examples\basic.calc` works from the playground folder.

## Reading the output

**Parse tree**

```
stmt (Let)  "let x = 2 + 3 * 4;"      <- rule name, (# label of the alternative that matched), source text
├── 'let'  @2:0                       <- literal token, @line:column
├── ID "x"  @2:4                      <- named token type + its text
├── expr (Add)  "2 + 3 * 4"
│   ├── expr (Num)  "2"
...
```

Cyan = parser rules, magenta = which labeled alternative matched,
yellow = literal tokens from the grammar (`'let'`), green = named tokens (`ID`).

**Errors** show where parsing went wrong, with the source line:

```
error [parser] 2:12  extraneous input '*' expecting {'(', NUMBER, ID}
    2 | let y = 3 * * 4;
      |             ^
```

`[lexer]` errors mean no token rule matched some characters. `[parser]` errors
mean the tokens were fine, but they appeared in an order the grammar doesn't allow.

## Visitor vs listener: two ways to write code against the tree

Both are generated from your grammar. `main.cpp` is where you use them.

- **Visitor** (`grammars/Calc/main.cpp`): you write `visitX()` methods that
  **return values**, and **you** decide when to visit children. Good for
  evaluating/interpreting: `visitAdd` returns `visit(left) + visit(right)`.
- **Listener** (`grammars/Ry/main.cpp`): you write `enterX()`/`exitX()` methods,
  and ANTLR's walker calls them as it walks the **whole** tree. Good for
  collecting info or checking things: "every label defined", "every branch target exists".

To see which methods and accessors you can use, open
`build/generated/<Name>/<Name>Parser.h` (the `...Context` classes) and
`<Name>BaseVisitor.h` / `<Name>BaseListener.h`.

## Making your own grammar

```
.\new Json
.\play Json examples\example.txt
```

That creates `grammars/Json/` with a starter grammar, a `main.cpp` with a
visitor, and an example input. The grammar name must start with an uppercase letter.

## Exercises

1. **Break things on purpose.** In `Calc.g4`, delete the `WS` rule, then remove
   `EOF` from `program`, then swap the `# Mul` and `# Add` lines. Rerun
   `.\play Calc examples\basic.calc` after each change and note what happens
   (restore each change before the next).
2. **Add subtraction and division** to Calc: grammar alternatives *and* `visitX`
   methods. Try `10 - 3 - 2`: does it give 5 (left-assoc) or 9?
3. **Watch left recursion.** `.\play Calc -e "1 + 2 * 3" --rule expr --trace`.
   Notice where `enter expr` happens when the parser sees `+`: that's the
   rewrite of `expr : expr '+' expr` into a loop.
4. **Add `if`/`while` to Calc**, with blocks `{ ... }` and comparison operators.
5. **Ry: make functions a real feature.** `Documentation.md` has
   `fn add(a, b) { return a + b; }` as an idea. Add it to `Ry.g4`, write
   examples, and extend the listener to check that every call has the right
   number of arguments.
6. **Ry: add a `# label` to every `expr`/`operand` alternative** and write a
   visitor that evaluates Ry expressions, like Calc does.
7. **Build an AST.** Write a Calc visitor that returns your own structs
   (`struct Add { Expr left, right; }`...) instead of numbers, then evaluate
   the AST. This is the shape your Rust interpreter would have.
