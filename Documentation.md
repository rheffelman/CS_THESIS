# Abstract
Code in my programming language is composed of a sequence of statements. Statements are executed sequentially and delimited by ";", semicolons, just like in C/C++/Rust.
Statements can be anything that the language knows how to interpret.

Types of valid statements:
1. Operations. An operation requires an object and an operator. For example: `i = 0;` is a valid statement assuming some variable i exists. You can make compound statements,
   using parentheses for order of operations, there is no implicit order of operations only explicit. For example: `Ts = (H * T1) + ((1.0 - H) * (T1 + T2))` works assuming all of those objects exist.
3. Declarations. For example: `let x = 5;` is a valid statement.
  
## Variables/Types
Types are inferred, and declared using the "let" keyword. the following types exist:
- Numeric
- String
- Vector
- Bool
- Unknown
- Null

Code example using variables:
```
let x = 5;
let pi = 3.14;
let y = x + pi;
```

## Labels
Labels are the programmers way of controlling the flow of control. Labels can be declared and branched to using "label" and "branch" keywords:
```
let i = 0;
let y = 10;

branch one;

label one:
    let hw = "Hello World!\n";
    print(hw);
    i = i + 1;
    if (i < y) branch one;
```

## If Statements
I'm aiming for if statements to behave essentially exactly like C++ or C if statements:
```
if (condition) {
  // execute code
}
```
where the code will be executed IF the condition is true, where a condition is a bool or a logical statement evaluatable to a single bool.

## Functions and Loops
Functions and loops are intentionally NOT a feature of the language. Their functionality can be implemented using labels and branching. I plan to add some syntax that'll make labels
that are serving as functions simpler. Something like:
```
let x = 6;
let y = 7;
let sum = 0;

sum = branch add(x,y);
label add(x, y):
  return x + y;
```


```
