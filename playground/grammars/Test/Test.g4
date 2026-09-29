grammar Test;

// a program is zero or more statements, followed by an end-of-file
program
    : statement* EOF
    ;

statement
    : expr ';'
    | KEYWORD '(' expr ')' block
    | KEYWORD expr ';'
    ;

block 
    : '{' statement* '}'
    ;

expr
    : operand (binaryOperator operand)?
    ;

operand
    : ID
    | NUMBER
    | ID '(' argList? ')'
    ;

binaryOperator : '+' | '-' | '*' | '/' | '<' | '>' | '==' | '=';

KEYWORD : 'let' | 'if';
ID     : [a-zA-Z_] [a-zA-Z_0-9]* ;   // names: start with a letter or _, then letters/digits/_
NUMBER : [0-9]+ ;
WS     : [ \t\r\n]+ -> skip ;
argList   : expr (',' expr)* ; // functionCall(arg1, arg2, arg3, arg4);