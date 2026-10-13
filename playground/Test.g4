grammar Test;

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
    | NUMBER
    | STRING
    | ID '(' argList? ')'
    | '(' expr ')'
    ;

binaryOperator : '+' | '-' | '*' | '/' | '<' | '>' | '==' | '=' | '!=' | 'NOT' | 'AND' | 'OR' | 'not' | 'and' | 'or';

ID     : [a-zA-Z_] [a-zA-Z_0-9]* ; 
STRING : '"' ( '\\' . | ~["\\\r\n] )* '"' ;
NUMBER : [0-9]+ ;
WS     : [ \t\r\n]+ -> skip ;
argList   : expr (',' expr)* ; // functionCall(arg1, arg2, arg3, arg4);