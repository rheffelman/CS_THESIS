// Starter ANTLR grammar for the .ry language described in Documentation.md.
// Parser rules are lowercase, lexer (token) rules are UPPERCASE.
grammar Ry;

// ---------- Parser rules ----------

program
    : statement* EOF
    ;

statement
    : 'let' ID '=' expr ';'                                   # letStmt
    | ID '=' expr ';'                                         # assignStmt
    | 'label' ID ('(' paramList? ')')? ':'                    # labelStmt
    | 'branch' ID ('(' argList? ')')? ';'                     # branchStmt
    | 'if' '(' expr ')' (block | 'branch' ID ';')             # ifStmt
    | 'return' expr? ';'                                      # returnStmt
    | expr ';'                                                # exprStmt
    ;

block
    : '{' statement* '}'
    ;

// No implicit order of operations: a binary expression is exactly
// `operand OP operand`. Anything more complex needs explicit parentheses,
// so `a + b * c` is a syntax error but `a + (b * c)` is fine.
expr
    : operand (binOp operand)?
    | 'branch' ID '(' argList? ')'                            // sum = branch add(x, y)
    ;

operand
    : '(' expr ')'
    | ID '(' argList? ')'                                     // function call, e.g. print(hw)
    | '[' argList? ']'                                        // vector literal
    | ID
    | literal
    ;

binOp
    : '+' | '-' | '*' | '/' | '<' | '>' | '<=' | '>=' | '==' | '!=' | '&&' | '||'
    ;

paramList : ID (',' ID)* ;
argList   : expr (',' expr)* ;

literal
    : NUMBER
    | STRING
    | 'true' | 'false'
    | 'null'
    ;

// ---------- Lexer rules ----------

NUMBER  : '-'? [0-9]+ ('.' [0-9]+)? ;
STRING  : '"' ( '\\' . | ~["\\\r\n] )* '"' ;
ID      : [a-zA-Z_] [a-zA-Z_0-9]* ;

LINE_COMMENT : '//' ~[\r\n]* -> skip ;
WS           : [ \t\r\n]+    -> skip ;
