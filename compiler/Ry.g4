grammar Ry;

program
    : statement* EOF
    ;

statement
    : expr ';' # ExprStatement
    | 'if' '(' expr ')' block # If
    | 'let' expr ';' # Let
    | 'label' expr block # Label
    | 'branch' expr ';' # Branch
    | 'fn' expr block # Function
    ;

block 
    : '{' statement* '}'
    ;

expr
    : operand (binaryOperator operand)?
    ;

operand
    : ID
    | '-'? NUMBER
    | '-'? FLOAT
    | STRING
    | BOOL
    | ID '(' argList? ')'
    | '(' expr ')'
    ;

binaryOperator : '+' | '-' | '*' | '/' | '<' | '>' | '==' | '=' | '!=' | 'NOT' | 'AND' | 'OR' | 'not' | 'and' | 'or';

BOOL   : 'true' | 'false' ; // before ID so true/false aren't read as names
ID     : [a-zA-Z_] [a-zA-Z_0-9]* ;
STRING : '"' ( '\\' . | ~["\\\r\n] )* '"' ;
FLOAT  : [0-9]+ '.' [0-9]* EXPONENT? // 1.0, 1., 1.5e-3
       | '.' [0-9]+ EXPONENT?        // .1
       | [0-9]+ EXPONENT             // 1e10
       ;
NUMBER : [0-9]+ ;
fragment EXPONENT : [eE] [+-]? [0-9]+ ;
WS     : [ \t\r\n]+ -> skip ;
argList   : expr (',' expr)* ; // functionCall(arg1, arg2, arg3, arg4);